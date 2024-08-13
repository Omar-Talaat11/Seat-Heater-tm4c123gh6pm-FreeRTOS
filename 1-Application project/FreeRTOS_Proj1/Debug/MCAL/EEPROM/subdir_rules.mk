################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
MCAL/EEPROM/%.obj: ../MCAL/EEPROM/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"E:/ti/ccs1270/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="D:/Autosar course/RTOS/RTOS_WS/FreeRTOS_Proj1" --include_path="D:/Autosar course/RTOS/RTOS_WS/FreeRTOS_Proj1/MCAL/ADC" --include_path="D:/Autosar course/RTOS/RTOS_WS/FreeRTOS_Proj1/MCAL/EEPROM" --include_path="D:/Autosar course/RTOS/FreeRTOS Essential Files/Common" --include_path="D:/Autosar course/RTOS/FreeRTOS Essential Files/MCAL" --include_path="D:/Autosar course/RTOS/FreeRTOS Essential Files/MCAL/GPIO" --include_path="D:/Autosar course/RTOS/FreeRTOS Essential Files/MCAL/GPTM" --include_path="D:/Autosar course/RTOS/FreeRTOS Essential Files/MCAL/UART" --include_path="D:/Autosar course/RTOS/FreeRTOSv202212.01/FreeRTOS/Source/include" --include_path="D:/Autosar course/RTOS/FreeRTOSv202212.01/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="E:/ti/ccs1270/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs="ccs" --define=PART_TM4C123GH6PM -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="MCAL/EEPROM/$(basename $(<F)).d_raw" --obj_directory="MCAL/EEPROM" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


