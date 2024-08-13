/**********************************************************************************************
 *
 * Module: GPIO
 *
 * File Name: GPIO.h
 *
 * Description: Header file for the TM4C123GH6PM DIO driver for TivaC Built-in Buttons and LEDs
 *
 * Author: Edges for Training Team
 *
 ***********************************************************************************************/

#ifndef GPIO_H_
#define GPIO_H_

#include "std_types.h"



#define GPIO_PORTE_PRIORITY_MASK      0xFFFFFF1F
#define GPIO_PORTE_PRIORITY_BITS_POS  5
#define GPIO_PORTE_INTERRUPT_PRIORITY 5

#define GPIO_PORTF_PRIORITY_MASK      0xFF1FFFFF
#define GPIO_PORTF_PRIORITY_BITS_POS  21
#define GPIO_PORTF_INTERRUPT_PRIORITY 5

#define PRESSED                ((uint8)0x00)
#define RELEASED               ((uint8)0x01)

void GPIO_BuiltinButtonsLedsInit(void);
void GPIO_ExternalButtonsLedsInit(void);
void GPIO_ADCPD0D1Init(void);

void GPIO_RedLedOn(void);
void GPIO_BlueLedOn(void);
void GPIO_GreenLedOn(void);

void GPIO_RedLedOff(void);
void GPIO_BlueLedOff(void);
void GPIO_GreenLedOff(void);

void GPIO_ExRedLedOn(void);
void GPIO_ExBlueLedOn(void);
void GPIO_ExGreenLedOn(void);

void GPIO_ExRedLedOff(void);
void GPIO_ExBlueLedOff(void);
void GPIO_ExGreenLedOff(void);

void GPIO_RedLedToggle(void);
void GPIO_BlueLedToggle(void);
void GPIO_GreenLedToggle(void);

uint8 GPIO_SW1GetState(void);
uint8 GPIO_SW2GetState(void);
uint8 GPIO_ExSWGetState(void);

void GPIO_SW1EdgeTriggeredInterruptInit(void);
void GPIO_SW2EdgeTriggeredInterruptInit(void);
void GPIO_ExSWEdgeTriggeredInterruptInit(void);



#endif /* GPIO_H_ */
