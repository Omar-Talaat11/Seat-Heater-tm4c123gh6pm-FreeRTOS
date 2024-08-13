 /******************************************************************************
 *
 * Module: EEPROM
 *
 * File Name: EEPROM.c
 *
 * Description: Source file for the EEPROM Driver for TivaC.
 *
 * Author: Omar Talaat
 *
 *******************************************************************************/


#include "EEPROM.h"
#include "tm4c123gh6pm_registers.h"

typedef struct
{
    char* Info;
    uint32 TimeStamp;
} DiagnosticsTaskInformation; /* Struct to Carry Information of Diagnostic Task */

void EEPROM_Init(void)
{
    SYSCTL_RCGCEEPROM_REG |= 0x01;
    while(!(SYSCTL_PREEPROM_REG & 0x01));

    while(EEPROM_EEDONE_REG & 0x01);

    if((EEPROM_EESUPP_REG&0x04) | (EEPROM_EESUPP_REG&0x08)) while(1);

    while(EEPROM_EEDONE_REG & 0x01);
    EEPROM_EEBLOCK_REG = 0x00;
    while(EEPROM_EEDONE_REG & 0x01);
    EEPROM_EEOFFSET_REG = 0x00;
}

void EEPROM_SaveBlock0(void*Data)
{
    DiagnosticsTaskInformation* RecievedData = (DiagnosticsTaskInformation*) Data;
    while(EEPROM_EEDONE_REG & 0x01);
    EEPROM_EERDWRINC_REG = (uint32)(RecievedData->Info);
    while(EEPROM_EEDONE_REG & 0x01);
    EEPROM_EERDWRINC_REG = (uint32)(RecievedData->TimeStamp);
}

void EEPROM_PointBeginBlock0(void)
{
    while(EEPROM_EEDONE_REG & 0x01);
    EEPROM_EEBLOCK_REG = 0x00;
    while(EEPROM_EEDONE_REG & 0x01);
    EEPROM_EEOFFSET_REG = 0x00;
}

void EEPROM_ReadBlock0(void*Data)
{
    DiagnosticsTaskInformation SentData;
    while(EEPROM_EEDONE_REG & 0x01);
    SentData.Info = (char*)EEPROM_EERDWRINC_REG;
    while(EEPROM_EEDONE_REG & 0x01);
    SentData.TimeStamp = EEPROM_EERDWRINC_REG;
    *((DiagnosticsTaskInformation*)Data) = SentData;
}

void EEPROM_SaveBlock1(void*Data)
{
    DiagnosticsTaskInformation* RecievedData = (DiagnosticsTaskInformation*) Data;
    static uint8 EEPROM_Write_Offset = 0x00;
    while(EEPROM_EEDONE_REG & 0x01);
    while(EEPROM_EEDONE_REG & 0x01);
    EEPROM_EEBLOCK_REG = 0x04;
    while(EEPROM_EEDONE_REG & 0x01);
    EEPROM_EEOFFSET_REG = EEPROM_Write_Offset;
    while(EEPROM_EEDONE_REG & 0x01);

    EEPROM_EERDWRINC_REG = (uint32)RecievedData->Info;
    while(EEPROM_EEDONE_REG & 0x01);
    EEPROM_EERDWRINC_REG = (uint32)RecievedData->TimeStamp;
    while(EEPROM_EEDONE_REG & 0x01);
    EEPROM_Write_Offset = EEPROM_EEOFFSET_REG & 0xF;
}

void EEPROM_ReadBlock1(void*Data)
{
    static uint8 EEPROM_Read_Offset = 0x00;
    EEPROM_EEBLOCK_REG = 0x04;
    EEPROM_EEOFFSET_REG = EEPROM_Read_Offset;

    DiagnosticsTaskInformation SentData;
    SentData.Info = (char*)EEPROM_EERDWRINC_REG;
    while(EEPROM_EEDONE_REG & 0x01);
    SentData.TimeStamp = EEPROM_EERDWRINC_REG;
    while(EEPROM_EEDONE_REG & 0x01);
    *((DiagnosticsTaskInformation*)Data) = SentData;

    EEPROM_Read_Offset = EEPROM_EEOFFSET_REG & 0xF;
}
