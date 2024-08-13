 /******************************************************************************
 *
 * Module: Main
 *
 * File Name: main.c
 *
 * Description: Source file for the main application for TivaC using FreeRTOS
 *
 * Author: Omar Talaat
 *
 *******************************************************************************/


/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

/* MCAL includes. */
#include "uart0.h"
#include "gpio.h"
#include "EEPROM.h"
#include "adc.h"
#include "tm4c123gh6pm_registers.h"

///////////////////////////        USED DEFINATIONS       ///////////////////////////

#define mainOFF_TEMP            0
#define mainLOW_TEMP            20
#define mainMED_TEMP            30
#define mainHIGH_TEMP           40

#define mainERROR_NO_INTENSITY         'n'
#define mainNO_INTENSITY               'N'
#define mainLOW_INTENSITY              'L'
#define mainMED_INTENSITY              'M'
#define mainHIGH_INTENSITY             'H'

/* Definitions for the event bits in the event group. */
#define mainSW1_PRESSED_BIT                 ( 1UL << 0UL )  /* SW1 event bit 0, which is set by button SW1 task. */
#define mainSW2_PRESSED_BIT                 ( 1UL << 1UL )  /* SW2 event bit 1, which is set by button SW2 task. */
#define mainERROR_OVER_DRIVER_BIT           ( 1UL << 2UL )  /* Event bit 2 set when Driver seat is over 40 Degrees */
#define mainERROR_UNDER_DRIVER_BIT          ( 1UL << 3UL )  /* Event bit 3 set when Driver seat is under 5 Degrees */
#define mainERROR_OVER_PASSENGER_BIT        ( 1UL << 4UL )  /* Event bit 4 set when Passenger seat is over 40 Degrees */
#define mainERROR_UNDER_PASSENGER_BIT       ( 1UL << 5UL )  /* Event bit 5 set when Passenger seat is under 5 Degrees */

///////////////////////////        QUEUES CREATED       ///////////////////////////

QueueHandle_t xDriverIntensityQueue;
QueueHandle_t xPassengerIntensityQueue;

QueueHandle_t xDriverDiagnosticsQueue;
QueueHandle_t xPassengerDiagnosticsQueue;

/////////////////////////// SEMAPHORES AND MUTEX CREATED ///////////////////////////

xSemaphoreHandle xDriverMutex;
xSemaphoreHandle xPassengerMutex;

xSemaphoreHandle xDriverTempSemphr;
xSemaphoreHandle xPassengerTempSemphr;

xSemaphoreHandle xDriverErrorSemaphore;
xSemaphoreHandle xPassengerErrorSemaphore;

///////////////////////////     EVENT GROUPS CREATED    ////////////////////////////

EventGroupHandle_t xEventGroup;

///////////////////////////     NEEDED TASK HANDLES    /////////////////////////////

xTaskHandle Button_Handle_Task;

xTaskHandle Driver_Intensity_Task;
xTaskHandle Passenger_Intensity_Task;

xTaskHandle Driver_Heater_Task;
xTaskHandle Passenger_Heater_Task;

xTaskHandle Driver_Temp_Task;
xTaskHandle Passenger_Temp_Task;

xTaskHandle Display_Task;

xTaskHandle Driver_Error_Task;
xTaskHandle Passenger_Error_Task;

xTaskHandle Diagnostic_Task;

xTaskHandle xTask0Handle;

/////////////////////////     NEEDED STRUCT TYPEDEFS    ///////////////////////////

typedef struct
{
    char* Info;
    uint32 TimeStamp;
} DiagnosticsTaskInformation; /* Struct to Carry Information of Diagnostic Task */

typedef struct
{
    void (*adcUsedFun)(void);
} TempInitConvTaskInformation; /* Struct to Carry Information of Init Conversion Task */

typedef struct
{
    xSemaphoreHandle* usedErrorSmphr;
    xTaskHandle *usedTask;
    xQueueHandle *usedQueue;
    void (*LedOnFun)(void);
    void (*LedOffFun)(void);
    EventBits_t UnderBit;
    EventBits_t OverBit;
    uint16* seatTemp;
} ErrorHandleTaskInformation; /* Struct to Carry Information of Error Handle Task */

/////////////////////////     NEEDED GLOBAL VARIABLES    ///////////////////////////

uint8 gdSeatTemp;
uint8 gpSeatTemp;

uint16 gDriverTemp;
uint16 gPassengerTemp;

char* gOverDriver =         "Driver Over 40";
char* gUnderDriver =        "Driver Below 5";
char* gOverPassenger =      "Passenger Over 40";
char* gUnderPassenger =     "Passenger Below 5";

DiagnosticsTaskInformation gSystemDriverLastState;
DiagnosticsTaskInformation gSystemPassengerLastState;

TempInitConvTaskInformation TempInitConvDriverTask = { ADC0_StartConv };
TempInitConvTaskInformation TempInitConvPassengerTask = { ADC1_StartConv };

ErrorHandleTaskInformation ErrorHandleDriverTask = { &xDriverErrorSemaphore,
                                                     &Driver_Intensity_Task,
                                                     &xDriverIntensityQueue,
                                                     GPIO_RedLedOn,
                                                     GPIO_RedLedOff,
                                                     mainERROR_UNDER_DRIVER_BIT,
                                                     mainERROR_OVER_DRIVER_BIT,
                                                     &gDriverTemp
                                                    };
ErrorHandleTaskInformation ErrorHandlePassengerTask =  { &xPassengerErrorSemaphore,
                                                         &Passenger_Intensity_Task,
                                                         &xPassengerIntensityQueue,
                                                         GPIO_ExRedLedOn,
                                                         GPIO_ExRedLedOff,
                                                         mainERROR_UNDER_PASSENGER_BIT,
                                                         mainERROR_OVER_PASSENGER_BIT,
                                                         &gPassengerTemp
                                                        };

uint32 ullTasksOutTime[13];
uint32 ullTasksInTime[13];
uint32 ullTasksExecutionTime[13];

uint32 ullSmphrGiveTime[6];
uint32 ullSmphrTakeTime[6];
uint32 ullSmphrAquireTime[6];

///////////////////////////     FUNCTIONS USED PROTOTYPES    ////////////////////////////

/*
 * The HW setup function
 */
static void prvSetupHardware(void);

void vButtonHandleTask(void *pvParameters);

void vDriverHeaterControlTask(void *pvParameters);
void vPassengerHeaterControlTask(void *pvParameters);

void vDriverIntensityControlTask(void *pvParameters);
void vPassengerIntensityControlTask(void *pvParameters);

void vTempReadTask(void *pvParameters);

void vTempinitConv(void *pvParameters);

void vDisplayUserTask(void *pvParameters);

void vErrorHandleTask(void *pvParameters);

void vDiagnosticsTask(void *pvParameters);

void vRunTimeMeasurementsTask(void *pvParameters);


int main()
{
    prvSetupHardware();
    ///////////////////////////     EVENT GROUPS     ///////////////////////////

    xEventGroup = xEventGroupCreate();

    /////////////////////////// SEMAPHORES AND MUTEX ///////////////////////////

    /* Mutex to Protect Shared Resource of The Required Seat Temperature */
    xDriverMutex = xSemaphoreCreateMutex();                 vQueueSetQueueNumber(xDriverMutex,0);
    xPassengerMutex = xSemaphoreCreateMutex();              vQueueSetQueueNumber(xPassengerMutex,1);

    /* Semaphore to Protect Shared Resource of The Current Seat Temperature */
    xDriverTempSemphr = xSemaphoreCreateBinary();           vQueueSetQueueNumber(xDriverTempSemphr,2);
    xPassengerTempSemphr = xSemaphoreCreateBinary();        vQueueSetQueueNumber(xPassengerTempSemphr,3);

    /* Give The Semaphore Initially */
    xSemaphoreGive(xPassengerTempSemphr);
    xSemaphoreGive(xDriverTempSemphr);

    /* Semaphore to Create Synchornization of Error Task in Case Temperature is Out of Range */
    xDriverErrorSemaphore = xSemaphoreCreateBinary();       vQueueSetQueueNumber(xDriverErrorSemaphore,4);
    xPassengerErrorSemaphore = xSemaphoreCreateBinary();    vQueueSetQueueNumber(xPassengerErrorSemaphore,5);

    ///////////////////////////        QUEUES       ///////////////////////////

    /* Create a queue capable of containing 3 uint8 values to exchange Intensity Information. */
    xDriverIntensityQueue = xQueueCreate(3, sizeof(uint8));
    xPassengerIntensityQueue = xQueueCreate(3, sizeof(uint8));

    /* Create a queue capable of containing 10 Diagnostic values to save the Diagnostic Information. */
    xDriverDiagnosticsQueue = xQueueCreate(10,sizeof(DiagnosticsTaskInformation));
    xPassengerDiagnosticsQueue = xQueueCreate(10,sizeof(DiagnosticsTaskInformation));

    ///////////////////////////        TASKS       ///////////////////////////

    /* Handle The Button Press Task. */
    xTaskCreate(vButtonHandleTask, "Button Task", 64, NULL, 5, &Button_Handle_Task);

    /* Control The Intensity of The Heater Task. */
    xTaskCreate(vDriverIntensityControlTask, "Driver Intensity Control Task", 128, NULL, 1, &Driver_Intensity_Task);
    xTaskCreate(vPassengerIntensityControlTask, "Passenger Intensity Control Task", 128, NULL, 1, &Passenger_Intensity_Task);

    /* Control The Output to The Heater Task. */
    xTaskCreate(vDriverHeaterControlTask, "Driver Heater Task", 64, NULL, 2, &Driver_Heater_Task);
    xTaskCreate(vPassengerHeaterControlTask, "Passenger Heater Task", 64, NULL, 2, &Passenger_Heater_Task);

    /* Initiate The ADC Conversion to Read The Temperature Task. */
    xTaskCreate(vTempinitConv, "Driver Temp Read Task", 64, (void*)&TempInitConvDriverTask, 3, &Driver_Temp_Task);
    xTaskCreate(vTempinitConv, "Passenger Temp Read Task", 64, (void*)&TempInitConvPassengerTask, 3, &Passenger_Temp_Task);

    /* Display The Needed Information to The User Task. */
    xTaskCreate(vDisplayUserTask, "Display User Task", 128, NULL, 2, &Display_Task);

    /* Handle The Errors Task. */
    xTaskCreate(vErrorHandleTask, "Driver Handle Error Task", 64, (void*)&ErrorHandleDriverTask, 5, &Driver_Error_Task);
    xTaskCreate(vErrorHandleTask, "Passenger Handle Error Task", 64, (void*)&ErrorHandlePassengerTask, 5, &Passenger_Error_Task);

    /*Save The Diagnostics Information Task. */
    xTaskCreate(vDiagnosticsTask, "Diagnostic Task", 128, NULL, 4, &Diagnostic_Task);

//    xTaskCreate(vRunTimeMeasurementsTask, "Run time", 64, NULL, 1, &xTask0Handle);

    ///////////////////////////        TAGS       ///////////////////////////

    vTaskSetApplicationTaskTag( Button_Handle_Task, ( TaskHookFunction_t ) 1 );

    vTaskSetApplicationTaskTag( Driver_Intensity_Task, ( TaskHookFunction_t ) 2 );
    vTaskSetApplicationTaskTag( Passenger_Intensity_Task, ( TaskHookFunction_t ) 3 );

    vTaskSetApplicationTaskTag( Driver_Heater_Task, ( TaskHookFunction_t ) 4 );
    vTaskSetApplicationTaskTag( Passenger_Heater_Task, ( TaskHookFunction_t ) 5 );

    vTaskSetApplicationTaskTag( Driver_Temp_Task, ( TaskHookFunction_t ) 6 );
    vTaskSetApplicationTaskTag( Passenger_Temp_Task, ( TaskHookFunction_t ) 7 );

    vTaskSetApplicationTaskTag( Display_Task, ( TaskHookFunction_t ) 8 );

    vTaskSetApplicationTaskTag( Driver_Error_Task, ( TaskHookFunction_t ) 9 );
    vTaskSetApplicationTaskTag( Passenger_Error_Task, ( TaskHookFunction_t ) 10 );

    vTaskSetApplicationTaskTag( xTask0Handle, ( TaskHookFunction_t ) 11 );

    vTaskSetApplicationTaskTag( Diagnostic_Task, ( TaskHookFunction_t ) 12 );

    /* Start the scheduler so the created tasks start executing. */
    vTaskStartScheduler();

    /* The following line should never be reached. */
    for (;;);

}

static void prvSetupHardware(void)
{
    /* Place here any needed HW initialization such as GPIO, UART, etc.  */
    UART0_Init();
    GPIO_BuiltinButtonsLedsInit();
    GPIO_ExternalButtonsLedsInit();
    GPIO_SW1EdgeTriggeredInterruptInit();
    GPIO_SW2EdgeTriggeredInterruptInit();
    GPIO_ExSWEdgeTriggeredInterruptInit();
    GPIO_ADCPD0D1Init();
    ADC_PD0D1Init();
    GPTM_WTimer0Init();
    EEPROM_Init();
}

void ADC0_Handler(void)
{
    BaseType_t xHigherPriorityTaskWoken1 = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken2 = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken3 = pdFALSE;

    xSemaphoreTakeFromISR(xDriverTempSemphr,&xHigherPriorityTaskWoken1);

    gDriverTemp = ADC_PD0Read()*45/4095;

    xSemaphoreGiveFromISR(xDriverTempSemphr,&xHigherPriorityTaskWoken2);

    if(gDriverTemp<5 || gDriverTemp>40)
    {
        xSemaphoreGiveFromISR(xDriverErrorSemaphore,&xHigherPriorityTaskWoken3);
    }
    ADC0_ADCISC_REG = 0x1;
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken1 | xHigherPriorityTaskWoken2 | xHigherPriorityTaskWoken3 );
}

void ADC1_Handler(void)
{
    BaseType_t xHigherPriorityTaskWoken1 = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken2 = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken3 = pdFALSE;

    xSemaphoreTakeFromISR(xPassengerTempSemphr,&xHigherPriorityTaskWoken1);

    gPassengerTemp = ADC_PD1Read()*45/4095;

    xSemaphoreGiveFromISR(xPassengerTempSemphr,&xHigherPriorityTaskWoken2);

    if(gPassengerTemp<5 || gPassengerTemp>40)
    {
        xSemaphoreGiveFromISR(xPassengerErrorSemaphore,&xHigherPriorityTaskWoken3);
    }
    ADC1_ADCISC_REG = 0x1;
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken1 | xHigherPriorityTaskWoken2 | xHigherPriorityTaskWoken3 );
}

void vTempinitConv(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    TempInitConvTaskInformation * pTaskInformation = (TempInitConvTaskInformation *) pvParameters;

    for(;;)
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 500 ) );
        pTaskInformation->adcUsedFun();
    }
}

void GPIOPortF_Handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(GPIO_PORTF_RIS_REG & (1<<0))           /* PF0 handler code */
    {
        xEventGroupSetBitsFromISR(xEventGroup, mainSW1_PRESSED_BIT,&xHigherPriorityTaskWoken);
        GPIO_PORTF_ICR_REG   |= (1<<0);       /* Clear Trigger flag for PF0 (Interrupt Flag) */
    }
    else if(GPIO_PORTF_RIS_REG & (1<<4))      /* PF4 handler code */
    {
        xEventGroupSetBitsFromISR(xEventGroup, mainSW1_PRESSED_BIT,&xHigherPriorityTaskWoken);
        GPIO_PORTF_ICR_REG   |= (1<<4);       /* Clear Trigger flag for PF4 (Interrupt Flag) */
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void GPIOPortE_Handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xEventGroupSetBitsFromISR(xEventGroup, mainSW2_PRESSED_BIT,&xHigherPriorityTaskWoken);
    GPIO_PORTE_ICR_REG |= (1<<4);


    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


void vButtonHandleTask(void *pvParameters)
{
    uint8 gDriverButtonCount = 0;
    uint8 gPassengerButtonCount = 0;
    EventBits_t xEventGroupValue;
    const EventBits_t xBitsToWaitFor = ( mainSW1_PRESSED_BIT | mainSW2_PRESSED_BIT);
    for(;;)
    {
        /* Block to wait for event bits to become set within the event group. */
        xEventGroupValue = xEventGroupWaitBits( xEventGroup,     /* The event group to read. */
                                                xBitsToWaitFor,  /* Bits to test. */
                                                pdTRUE,          /* Clear bits on exit if the unblock condition is met. */
                                                pdFALSE,         /* Don't Wait for all bits. */
                                                portMAX_DELAY);  /* max timeout. */



        /* Check which events are set and take an action based on it. */
        if (xEventGroupValue & mainSW1_PRESSED_BIT)
        {
            if (xSemaphoreTake(xDriverMutex, portMAX_DELAY) == pdTRUE) {
                gDriverButtonCount++;
                gDriverButtonCount = gDriverButtonCount % 4;
                switch(gDriverButtonCount)
                {
                case(0):
                                        gdSeatTemp = mainOFF_TEMP;
                break;
                case(1):
                                        gdSeatTemp = mainLOW_TEMP;
                break;
                case(2):
                                        gdSeatTemp = mainMED_TEMP;
                break;
                case(3):
                                        gdSeatTemp = mainHIGH_TEMP;
                break;
                }

                xSemaphoreGive(xDriverMutex);
            }
        }
        else if (xEventGroupValue & mainSW2_PRESSED_BIT)
        {
            if (xSemaphoreTake(xPassengerMutex, portMAX_DELAY) == pdTRUE) {
                gPassengerButtonCount++;
                gPassengerButtonCount = gPassengerButtonCount % 4;
                switch(gPassengerButtonCount)
                {
                case(0):
                                        gpSeatTemp = mainOFF_TEMP;
                break;
                case(1):
                                        gpSeatTemp = mainLOW_TEMP;
                break;
                case(2):
                                        gpSeatTemp = mainMED_TEMP;
                break;
                case(3):
                                        gpSeatTemp = mainHIGH_TEMP;
                break;
                }
                xSemaphoreGive(xPassengerMutex);
            }
        }
        else
        {
            /* Do Nothing */
        }
    }
}


//void vTempReadTask(void *pvParameters)
//{
//    TickType_t xLastWakeTime = xTaskGetTickCount();
//
//    TempReadTaskInformation * pTaskInformation = (TempReadTaskInformation *) pvParameters;
//
//    for(;;)
//    {
//        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 400 ) );
//        xSemaphoreTake(*(pTaskInformation->usedMutex),portMAX_DELAY);
//        *(pTaskInformation->reqSeatTemp) = ((pTaskInformation->adcUsed)()*45/4095);
//        xSemaphoreGive(*(pTaskInformation->usedMutex));
//        if(((pTaskInformation->adcUsed)()*45/4095)<5 || ((pTaskInformation->adcUsed)()*45/4095)>40)
//        {
//            xSemaphoreGive(*(pTaskInformation->usedErrorSmphr));
//        }
//    }
//}


void vDriverIntensityControlTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8 intensity;

    for(;;)
    {

        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 200 ) );
        xSemaphoreTake(xDriverMutex, portMAX_DELAY);
        xSemaphoreTake(xDriverTempSemphr,portMAX_DELAY);
        if(gDriverTemp+10<=gdSeatTemp)
        {
            intensity = mainHIGH_INTENSITY;
        }
        else if(gDriverTemp+5<=gdSeatTemp)
        {
            intensity = mainMED_INTENSITY;
        }
        else if(gDriverTemp+2<=gdSeatTemp)
        {
            intensity = mainLOW_INTENSITY;
        }
        else
        {
            intensity = mainNO_INTENSITY;
        }
        xQueueSend(xDriverIntensityQueue, &intensity, portMAX_DELAY);
        xSemaphoreGive(xDriverTempSemphr);
        xSemaphoreGive(xDriverMutex);
    }
}


void vPassengerIntensityControlTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8 intensity;

    for(;;)
    {

        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 200 ) );
        xSemaphoreTake(xPassengerMutex, portMAX_DELAY);
        xSemaphoreTake(xPassengerTempSemphr,portMAX_DELAY);
        if(gPassengerTemp+10<=gpSeatTemp)
        {
            intensity = mainHIGH_INTENSITY;
        }
        else if(gPassengerTemp+5<=gpSeatTemp)
        {
            intensity = mainMED_INTENSITY;
        }
        else if(gPassengerTemp+2<=gpSeatTemp)
        {
            intensity = mainLOW_INTENSITY;
        }
        else
        {
            intensity = mainNO_INTENSITY;
        }
        xQueueSend(xPassengerIntensityQueue, &intensity, portMAX_DELAY);
        xSemaphoreGive(xPassengerTempSemphr);
        xSemaphoreGive(xPassengerMutex);
    }
}



void vDriverHeaterControlTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8 intensity;
    for (;;)
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 200 ) );

        xQueueReceive(xDriverIntensityQueue, &intensity, portMAX_DELAY);

        switch(intensity)
        {
        case(mainERROR_NO_INTENSITY):
        GPIO_BlueLedOff();
        GPIO_GreenLedOff();
        break;
        case(mainNO_INTENSITY):
        GPIO_BlueLedOff();
        GPIO_GreenLedOff();
        break;
        case(mainLOW_INTENSITY):
        GPIO_BlueLedOff();
        GPIO_GreenLedOn();
        break;
        case(mainMED_INTENSITY):
        GPIO_BlueLedOn();
        GPIO_GreenLedOff();
        break;
        case(mainHIGH_INTENSITY):
        GPIO_BlueLedOn();
        GPIO_GreenLedOn();
        break;
        }
    }
}

void vPassengerHeaterControlTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8 intensity;
    for (;;)
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 200 ) );

        xQueueReceive(xPassengerIntensityQueue, &intensity, portMAX_DELAY);

        switch(intensity)
        {
        case(mainERROR_NO_INTENSITY):
        GPIO_ExBlueLedOff();
        GPIO_ExGreenLedOff();
        break;
        case(mainNO_INTENSITY):
        GPIO_ExBlueLedOff();
        GPIO_ExGreenLedOff();
        break;
        case(mainLOW_INTENSITY):
        GPIO_ExBlueLedOff();
        GPIO_ExGreenLedOn();
        break;
        case(mainMED_INTENSITY):
        GPIO_ExBlueLedOn();
        GPIO_ExGreenLedOff();
        break;
        case(mainHIGH_INTENSITY):
        GPIO_ExBlueLedOn();
        GPIO_ExGreenLedOn();
        break;
        }
    }
}


void vDisplayUserTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8 dIntensity;
    uint8 pIntensity;
    for (;;)
    {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 1000 ) );
        xQueueReceive(xDriverIntensityQueue, &dIntensity, portMAX_DELAY);
        xQueueReceive(xPassengerIntensityQueue, &pIntensity, portMAX_DELAY);
        UART0_SendString("Driver:\r\nCurrent Temperature = ");
        UART0_SendInteger(gDriverTemp);
        UART0_SendString(" Degree\r\nRequired Heating Level = ");
        UART0_SendInteger(gdSeatTemp);
        UART0_SendString(" Degree\r\nThe Heater is Working with ");
        switch(dIntensity)
        {
        case(mainERROR_NO_INTENSITY):
                UART0_SendString("NO Intensity Because of Out of Range Error");
                break;
        case(mainNO_INTENSITY):
                UART0_SendString("NO Intensity");
                break;
        case(mainLOW_INTENSITY):
                UART0_SendString("LOW Intensity" );
                break;
        case(mainMED_INTENSITY):
                UART0_SendString("MEDIUM Intensity");
                break;
        case(mainHIGH_INTENSITY):
                UART0_SendString("HIGH Intensity");
                break;
        }
        UART0_SendString(" \r\n\n**************************************\r\n\n");
        UART0_SendString("Passenger:\r\nCurrent Temperature = ");
        UART0_SendInteger(gPassengerTemp);
        UART0_SendString(" Degree\r\nRequired Heating Level = ");
        UART0_SendInteger(gpSeatTemp);
        UART0_SendString(" Degree\r\nThe Heater is Working with ");
        switch(pIntensity)
        {
        case(mainERROR_NO_INTENSITY):
                UART0_SendString("NO Intensity Because of Out of Range Error");
                break;
        case(mainNO_INTENSITY):
                UART0_SendString("NO Intensity");
                break;
        case(mainLOW_INTENSITY):
                UART0_SendString("LOW Intensity" );
                break;
        case(mainMED_INTENSITY):
                UART0_SendString("MEDIUM Intensity");
                break;
        case(mainHIGH_INTENSITY):
                UART0_SendString("HIGH Intensity");
                break;
        }
        UART0_SendString(" \r\n\n**************************************\r\n\n");
    }
}



void vErrorHandleTask(void *pvParameters)
{
    uint8 newError = pdTRUE;
    ErrorHandleTaskInformation * pTaskInformation = (ErrorHandleTaskInformation *) pvParameters;

    xSemaphoreHandle usedErrSmphr = *(pTaskInformation->usedErrorSmphr);
    xTaskHandle Task_Handle = *(pTaskInformation->usedTask);
    xQueueHandle usedQueue = *(pTaskInformation->usedQueue);
    uint16* seatTemp = (pTaskInformation->seatTemp);

    xSemaphoreTake(usedErrSmphr,portMAX_DELAY);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8 no_intensity = mainERROR_NO_INTENSITY;
    for (;;)
    {
        vTaskSuspend(Task_Handle);
        xQueueSend(usedQueue, &no_intensity, portMAX_DELAY);
        (pTaskInformation->LedOnFun)();
        if(pdTRUE==newError)
        {
            if(*seatTemp<5)      xEventGroupSetBits(xEventGroup, pTaskInformation->UnderBit);
            if(*seatTemp>40)     xEventGroupSetBits(xEventGroup, pTaskInformation->OverBit);
            newError = pdFALSE;
        }
        if(*seatTemp>=5 && *seatTemp<=40)
        {
            vTaskResume(Task_Handle);
            (pTaskInformation->LedOffFun)();
            newError = pdTRUE;
            xSemaphoreTake(usedErrSmphr,portMAX_DELAY);
        }
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS( 200 ) );

    }
}

void vDiagnosticsTask(void *pvParameters)
{
    EventBits_t xEventGroupValue;
    const EventBits_t xBitsToWaitFor = ( mainERROR_OVER_DRIVER_BIT | mainERROR_UNDER_DRIVER_BIT |  mainERROR_OVER_PASSENGER_BIT | mainERROR_UNDER_PASSENGER_BIT );
    DiagnosticsTaskInformation ErrorInfo;
    for (;;)
    {
        xEventGroupValue = xEventGroupWaitBits( xEventGroup,     /* The event group to read. */
                                                xBitsToWaitFor,  /* Bits to test. */
                                                pdTRUE,          /* Clear bits on exit if the unblock condition is met. */
                                                pdFALSE,         /* Don't Wait for all bits. */
                                                pdMS_TO_TICKS(500));  /* max timeout. */
        ErrorInfo.TimeStamp = GPTM_WTimer0Read();
        if (xEventGroupValue & mainERROR_OVER_DRIVER_BIT)
        {
            ErrorInfo.Info = gOverDriver;
            EEPROM_SaveBlock1((void*)&ErrorInfo);
            xQueueSend(xDriverDiagnosticsQueue,&ErrorInfo,0);
        }
        else if(xEventGroupValue & mainERROR_UNDER_DRIVER_BIT)
        {
            ErrorInfo.Info = gUnderDriver;
            EEPROM_SaveBlock1((void*)&ErrorInfo);
            xQueueSend(xDriverDiagnosticsQueue,&ErrorInfo,0);
        }
        else if(xEventGroupValue & mainERROR_OVER_PASSENGER_BIT)
        {
            ErrorInfo.Info = gOverPassenger;
            EEPROM_SaveBlock1((void*)&ErrorInfo);
            xQueueSend(xPassengerDiagnosticsQueue,&ErrorInfo,0);
        }
        else if(xEventGroupValue & mainERROR_UNDER_PASSENGER_BIT)
        {
            ErrorInfo.Info = gUnderPassenger;
            EEPROM_SaveBlock1((void*)&ErrorInfo);
            xQueueSend(xPassengerDiagnosticsQueue,&ErrorInfo,0);
        }
        else
        {
            EEPROM_PointBeginBlock0();
            xQueueReceive(xDriverIntensityQueue, &(ErrorInfo.Info), portMAX_DELAY);
            EEPROM_SaveBlock0((void*)&ErrorInfo);
            gSystemDriverLastState = ErrorInfo;
            xQueueReceive(xPassengerIntensityQueue, &(ErrorInfo.Info), portMAX_DELAY);
            EEPROM_SaveBlock0((void*)&ErrorInfo);
            gSystemPassengerLastState = ErrorInfo;
            EEPROM_PointBeginBlock0();
        }
    }
}

void vRunTimeMeasurementsTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        uint8 ucCounter, ucCPU_Load;
        uint32 ullTotalTasksTime = 0;
        vTaskDelayUntil(&xLastWakeTime, 1000);
        for(ucCounter = 1; ucCounter < 13; ucCounter++)
        {
            ullTotalTasksTime += ullTasksExecutionTime[ucCounter];
        }
        ucCPU_Load = (ullTotalTasksTime * 100) /  GPTM_WTimer0Read();

        taskENTER_CRITICAL();
        UART0_SendString("CPU Load is ");
        UART0_SendInteger(ucCPU_Load);
        UART0_SendString("% \r\n");
        taskEXIT_CRITICAL();
    }
}


/*-----------------------------------------------------------*/
