################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32f427iihx.s 

C_SRCS += \
../Core/Startup/main.c 

OBJS += \
./Core/Startup/main.o \
./Core/Startup/startup_stm32f427iihx.o 

S_DEPS += \
./Core/Startup/startup_stm32f427iihx.d 

C_DEPS += \
./Core/Startup/main.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o Core/Startup/%.su Core/Startup/%.cyclo: ../Core/Startup/%.c Core/Startup/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/main.cyclo ./Core/Startup/main.d ./Core/Startup/main.o ./Core/Startup/main.su ./Core/Startup/startup_stm32f427iihx.d ./Core/Startup/startup_stm32f427iihx.o

.PHONY: clean-Core-2f-Startup

