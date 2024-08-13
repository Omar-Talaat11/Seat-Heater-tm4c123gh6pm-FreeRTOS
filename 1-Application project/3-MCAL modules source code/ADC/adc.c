 /******************************************************************************
 *
 * Module: ADC
 *
 * File Name: adc.c
 *
 * Description: Source file for the ADC Driver for TivaC.
 *
 * Author: Omar Talaat
 *
 *******************************************************************************/

#include "adc.h"
#include "tm4c123gh6pm_registers.h"

void ADC_PD0D1Init(void)
{
    SYSCTL_RCGCADC_REG |= 0x03;
    while(!(SYSCTL_PRADC_REG & 0x03));

    ADC0_ADCACTSS_REG &= 0x00;

    ADC0_ADCEMUX_REG &= 0x00;

    ADC0_ADCSSPRI_REG = 0x00;

    ADC0_ADCSSMUX0_REG = 0x07;

    ADC0_ADCSSCTL0_REG = 0x06;

    ADC0_ADCIM_REG |= 0x01;

    ADC0_ADCACTSS_REG |= 0x01;

    ADC0_ADCISC_REG = 0x01;

    NVIC_PRI3_REG = (NVIC_PRI3_REG & ADC0_PRIORITY_MASK) | (ADC0_INTERRUPT_PRIORITY<<ADC0_PRIORITY_BITS_POS);

    NVIC_EN0_REG    |= (1<<14);   /* Enable NVIC Interrupt for ADC0 by set bit number 14 in EN0 Register */

    ADC1_ADCACTSS_REG &= 0x00;

    ADC1_ADCEMUX_REG |= 0x00;

    ADC1_ADCSSPRI_REG = 0x00;

    ADC1_ADCSSMUX0_REG = 0x06;

    ADC1_ADCSSCTL0_REG = 0x06;

    ADC1_ADCIM_REG |= 0x01;

    ADC1_ADCACTSS_REG |= 0x01;

    ADC1_ADCISC_REG = 0x01;

    NVIC_PRI12_REG = (NVIC_PRI12_REG & ADC1_PRIORITY_MASK) | (ADC1_INTERRUPT_PRIORITY<<ADC1_PRIORITY_BITS_POS);

    NVIC_EN1_REG    |= (1<<16);   /* Enable NVIC Interrupt for ADC1 by set bit number 16 in EN1 Register */

}

uint16 ADC_PD0Read(void)
{
    return ADC0_ADCSSFIFO0_REG & 0x0FFF;
}

uint16 ADC_PD1Read(void)
{
    return ADC1_ADCSSFIFO0_REG & 0x0FFF;
}

void ADC0_StartConv(void)
{
    ADC0_ADCPSSI_REG|=1;
}

void ADC1_StartConv(void)
{
    ADC1_ADCPSSI_REG|=1;
}



