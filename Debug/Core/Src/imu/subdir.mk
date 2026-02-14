################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/imu/icm42688.c \
../Core/Src/imu/imu.c 

OBJS += \
./Core/Src/imu/icm42688.o \
./Core/Src/imu/imu.o 

C_DEPS += \
./Core/Src/imu/icm42688.d \
./Core/Src/imu/imu.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/imu/%.o Core/Src/imu/%.su Core/Src/imu/%.cyclo: ../Core/Src/imu/%.c Core/Src/imu/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-imu

clean-Core-2f-Src-2f-imu:
	-$(RM) ./Core/Src/imu/icm42688.cyclo ./Core/Src/imu/icm42688.d ./Core/Src/imu/icm42688.o ./Core/Src/imu/icm42688.su ./Core/Src/imu/imu.cyclo ./Core/Src/imu/imu.d ./Core/Src/imu/imu.o ./Core/Src/imu/imu.su

.PHONY: clean-Core-2f-Src-2f-imu

