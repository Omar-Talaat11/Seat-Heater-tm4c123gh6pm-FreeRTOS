 /******************************************************************************
 *
 * Module: ADC
 *
 * File Name: adc.h
 *
 * Description: Header file for the ADC Driver for TivaC.
 *
 * Author: Omar Talaat
 *
 *******************************************************************************/

#ifndef MCAL_ADC_ADC_H_
#define MCAL_ADC_ADC_H_

#include "std_types.h"

#define ADC0_PRIORITY_MASK      0xFF1FFFFF
#define ADC0_PRIORITY_BITS_POS  21
#define ADC0_INTERRUPT_PRIORITY 6

#define ADC1_PRIORITY_MASK      0xFFFFFF1F
#define ADC1_PRIORITY_BITS_POS  5
#define ADC1_INTERRUPT_PRIORITY 6

void ADC_PD0D1Init(void);

uint16 ADC_PD0Read(void);

uint16 ADC_PD1Read(void);

void ADC0_StartConv(void);

void ADC1_StartConv(void);




#endif /* MCAL_ADC_ADC_H_ */
