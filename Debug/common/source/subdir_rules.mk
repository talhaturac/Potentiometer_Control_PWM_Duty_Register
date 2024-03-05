################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
common/source/%.obj: ../common/source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1100/ccs/tools/compiler/ti-cgt-c2000_21.6.0.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 --include_path="C:/Users/90545/Desktop/CCS_11/potentiometer_control_pwm_duty_register/potentiometer_control_register_level" --include_path="C:/Users/90545/Desktop/CCS_11/potentiometer_control_pwm_duty_register/potentiometer_control_register_level/common/include" --include_path="C:/Users/90545/Desktop/CCS_11/potentiometer_control_pwm_duty_register/potentiometer_control_register_level/headers/include" --include_path="C:/Users/90545/Desktop/CCS_11/potentiometer_control_pwm_duty_register/potentiometer_control_register_level/driverlib" --include_path="C:/ti/ccs1100/ccs/tools/compiler/ti-cgt-c2000_21.6.0.LTS/include" --advice:performance=all --define=CPU1 --define=_DUAL_HEADERS --define=_DEBUG -g --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="common/source/$(basename $(<F)).d_raw" --obj_directory="common/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

common/source/%.obj: ../common/source/%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1100/ccs/tools/compiler/ti-cgt-c2000_21.6.0.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 --include_path="C:/Users/90545/Desktop/CCS_11/potentiometer_control_pwm_duty_register/potentiometer_control_register_level" --include_path="C:/Users/90545/Desktop/CCS_11/potentiometer_control_pwm_duty_register/potentiometer_control_register_level/common/include" --include_path="C:/Users/90545/Desktop/CCS_11/potentiometer_control_pwm_duty_register/potentiometer_control_register_level/headers/include" --include_path="C:/Users/90545/Desktop/CCS_11/potentiometer_control_pwm_duty_register/potentiometer_control_register_level/driverlib" --include_path="C:/ti/ccs1100/ccs/tools/compiler/ti-cgt-c2000_21.6.0.LTS/include" --advice:performance=all --define=CPU1 --define=_DUAL_HEADERS --define=_DEBUG -g --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="common/source/$(basename $(<F)).d_raw" --obj_directory="common/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


