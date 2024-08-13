 /******************************************************************************
 *
 * Module: EEPROM
 *
 * File Name: EEPROM.h
 *
 * Description: Header file for the EEPROM Driver for TivaC.
 *
 * Author: Omar Talaat
 *
 *******************************************************************************/

#ifndef MCAL_EEPROM_EEPROM_H_
#define MCAL_EEPROM_EEPROM_H_



void EEPROM_Init(void);

void EEPROM_SaveBlock0(void*Data);

void EEPROM_ReadBlock0(void*Data);

void EEPROM_PointBeginBlock0(void);

void EEPROM_SaveBlock1(void*Data);

void EEPROM_ReadBlock1(void*Data);

#endif /* MCAL_EEPROM_EEPROM_H_ */
