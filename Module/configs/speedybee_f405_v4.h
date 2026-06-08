#pragma once

/**
 * @brief Target identify
 */
#define FC_TARGET_MCU   STM32F405
#define BOARD_NAME      SPEEDYBEEF405V4
#define MANUFACTURER_ID SPEEDYBEE


/**
 * @brief Feature enable
 */
#define USE_ACC
#define USE_GYRO
#define USE_GYRO_SPI_ICM42688P
#define USE_ACC_SPI_ICM42688P
#define USE_TEMP_SPI_ICM42688P

#define USE_BARO
#define USE_BARO_DPS310

#define USE_SDCARD
#define USE_SDCARD_SPI

#define USE_MAX7456


/**
 * @brief Motor / servo / timer-capable pins definition
 */
#define MOTOR1_PIN PB6
#define MOTOR2_PIN PB7
#define MOTOR3_PIN PB0
#define MOTOR4_PIN PB1
#define MOTOR5_PIN PC8
#define MOTOR6_PIN PC9
#define MOTOR7_PIN PB10
#define MOTOR8_PIN PA15

#define SERVO1_PIN    PB15
#define RX_PPM_PIN    PA3
#define LED_STRIP_PIN PA8

#define CAMERA_CONTROL_PIN PB14

#define TIMER_PIN_MAPPING         \
    TIMER_PIN_MAP(0, PB6, 1, 0)   \
    TIMER_PIN_MAP(1, PB7, 1, 0)   \
    TIMER_PIN_MAP(2, PB0, 2, 0)   \
    TIMER_PIN_MAP(3, PB1, 2, 0)   \
    TIMER_PIN_MAP(4, PC8, 2, 0)   \
    TIMER_PIN_MAP(5, PC9, 2, 0)   \
    TIMER_PIN_MAP(6, PB10, 1, 0)  \
    TIMER_PIN_MAP(7, PA15, 1, 0)  \
    TIMER_PIN_MAP(8, PB15, 3, -1) \
    TIMER_PIN_MAP(9, PB14, 3, -1) \
    TIMER_PIN_MAP(10, PA8, 1, 0)  \
    TIMER_PIN_MAP(11, PA3, 3, -1)


/**
 * @brief UART pins
 */

#define UART1_TX_PIN PA9
#define UART1_RX_PIN PA10

#define UART2_TX_PIN PA2
#define UART2_RX_PIN PA3

#define UART3_TX_PIN PC10
#define UART3_RX_PIN PC11

#define UART4_TX_PIN PA0
#define UART4_RX_PIN PA1

#define UART5_RX_PIN PD2

#define UART6_TX_PIN PC6
#define UART6_RX_PIN PC7


/**
 * @brief I2C pins and devices
 */
#define I2C1_SCL_PIN PB8
#define I2C1_SDA_PIN PB9

#define MAG_I2C_INSTANCE  I2CDEV_1
#define BARO_I2C_INSTANCE I2CDEV_1


/**
 * @brief SPI pins
 */
#define SPI1_SCK_PIN PA5
#define SPI1_SDI_PIN PA6
#define SPI1_SDO_PIN PA7

#define SPI2_SCK_PIN PB13
#define SPI2_SDI_PIN PC2
#define SPI2_SDO_PIN PC3

#define SPI3_SCK_PIN PB3
#define SPI3_SDI_PIN PB4
#define SPI3_SDO_PIN PB5


/**
 * @brief SPI devices
 */
#define GYRO_1_SPI_INSTANCE SPI1
#define GYRO_1_CS_PIN       PA4
#define GYRO_1_EXTI_PIN     PC4
#define GYRO_1_ALIGN        CW90_DEG

#define MAX7456_SPI_INSTANCE SPI2
#define MAX7456_SPI_CS_PIN   PB12

#define SDCARD_SPI_INSTANCE SPI3
#define SDCARD_SPI_CS_PIN   PC14

/**
 * @brief LEDS, beeper, PINIO
 */
#define BEEPER_PIN PC15
#define LED0_PIN   PC13

#define PINIO1_PIN    PB11
#define PINIO1_CONFIG 129
#define PINIO1_BOX    0

#define BEEPER_INVERTED
#define SDCARD_DETECT_INVERTED


/**
 * @brief ADC channels
 */
#define ADC_VBAT_PIN PC0
#define ADC_CURR_PIN PC1
#define ADC_RSSI_PIN PC5

#define ADC1_DMA_OPT 0


/**
 * @brief System
 */
#define SYSTEM_HSE_MHZ 8

/**
 * @brief Default configuration
 */
#define DEFAULT_BLACKBOX_DEVICE BLACKBOX_DEVICE_SDCARD

#define DEFAULT_DSHOT_BURST   DSHOT_DMAR_OFF
#define DEFAULT_DSHOT_BITBANG DSHOT_BITBANG_OFF

#define DEFAULT_CURRENT_METER_SOURCE CURRENT_METER_ADC
#define DEFAULT_VOLTAGE_METER_SOURCE VOLTAGE_METER_ADC
#define DEFAULT_CURRENT_METER_SCALE  400


/**
 * @brief Default Serial Assignments
 */
#define SERIALRX_UART   SERIAL_PORT_USART2
#define MSP_UART        SERIAL_PORT_UART4
#define ESC_SENSOR_UART SERIAL_PORT_UART5