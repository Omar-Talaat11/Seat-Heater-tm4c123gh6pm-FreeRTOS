# Real-Time Seat Heater Control System Using FreeRTOS and TM4C123GH6PM
Project Overview:

Each seat includes:

- A button used to take the input level required to set the seat temperature.

- A temperature sensor (or potentiometer) to monitor the temperature.

- A heating element (represented by LEDs) to control the temperature based on the desired level using variable intensity power.

- A shared screen between both seats displays the current state of the system via UART.

System Features:

1- The system consists of 7 tasks, some reused in 2 different instances.

2- Shared resources are protected using semaphores and mutexes.

3- Event-based tasks are managed using event flags.

4- Data sharing between different tasks is handled through queues.

5- High responsiveness to buttons is achieved through interrupts to minimize CPU load.

6- ADC triggers an interrupt when conversion is finished.

7- Diagnostic data is stored in non-volatile EEPROM memory for future inspection.

Manual Runtime Measurements is done with the following Performance Metrics:

- CPU Load: 34%

- Tasks execution time and Resource lock time per task for each resource are documented in the project documentation.



MCAL Modules Developed and Used:

GPIO, UART, NVIC, GPTM, ADC, and EEPROM
