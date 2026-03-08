/*
 * icm42688_core.h
 *
 *  Created on: Dec 22, 2025
 *      Author: dobao
 */

#ifndef INC_ICM42688_H_
#define INC_ICM42688_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "imu/icm42688_registers.h"
#include "imu/icm42688_masks.h"
#include "stm32f4xx_hal.h"
#include "spi.h"


/*
 * =============================================================
 * 						ICM42688-P MACROS
 * =============================================================
 */
#define ICM42688_VDD				1800U	//In mV


/* Frequencies are declared in Hz */
#define ICM42688_SPI_MAX_CLKFREQ	24000000U
#define ICM42688_I2C_MAX_CLKFREQ	1000000U
#define ICM42688_MAX_RTC_FREQ		50000U
#define ICM42688_TYP_RTC_FREQ		32000U
#define ICM42688_MIN_RTC_FREQ		31000U


/* Other Defines */
#define ICM42688_SENSITIVITY_SCALE_FACTOR	32768.0f
#define ICM42688_SPI_TIMEOUT_MS				20U		//10ms
#define ICM42688_SPI_ADDR_MASK				0x7FU
#define ICM42688_SPI_READ_BIT				0x80U
#define ICM42688_WHO_AM_I_DEFAULT			0x47U




/*
 * ============================================================================
 * 								GYROSCOPE DEFINES
 * ============================================================================
 */
typedef enum{
	GYRO_ODR_32KHz		= (uint8_t)0x01,
	GYRO_ODR_16KHz 		= (uint8_t)0x02,
	GYRO_ODR_8KHz		= (uint8_t)0x03,	//Recommended due to widely used
	GYRO_ODR_4KHz		= (uint8_t)0x04,
	GYRO_ODR_2KHz		= (uint8_t)0x05,
	GYRO_ODR_1KHz		= (uint8_t)0x06,	//Default
	GYRO_ODR_200Hz		= (uint8_t)0x07,
	GYRO_ODR_100Hz		= (uint8_t)0x08,
	GYRO_ODR_50Hz		= (uint8_t)0x09,
	GYRO_ODR_25Hz		= (uint8_t)0x0A,
	GYRO_ODR_12Hz5		= (uint8_t)0x0B,
	GYRO_ODR_6Hz25		= (uint8_t)0x0C,	//Low Power Mode & Accel Only
	GYRO_ODR_3Hz125		= (uint8_t)0x0D,	//Low Power Mode & Accel Only
	GYRO_ODR_1Hz5625	= (uint8_t)0x0E,	//Low Power Mode & Accel Only
	GYRO_ODR_500Hz		= (uint8_t)0x0F,
}ICM42688_GyroODR_t;


typedef enum{
	GYRO_FSR_2000dps	= (uint8_t)0x00,
	GYRO_FSR_1000dps	= (uint8_t)0x01,
	GYRO_FSR_500dps 	= (uint8_t)0x02,
	GYRO_FSR_250dps		= (uint8_t)0x03,
	GYRO_FSR_125dps		= (uint8_t)0x04,
	GYRO_FSR_65dps5		= (uint8_t)0x05,
	GYRO_FSR_31dps25	= (uint8_t)0x06,
	GYRO_FSR_15dps625	= (uint8_t)0x07
}ICM42688_GyroFSR_t;


typedef enum{
	GYRO_NOTCHBW_11449Hz	= (uint8_t)0x00,
	GYRO_NOTCHBW_680Hz		= (uint8_t)0x01,
	GYRO_NOTCHBW_329Hz		= (uint8_t)0x02,
	GYRO_NOTCHBW_162Hz		= (uint8_t)0x03,
	GYRO_NOTCHBW_80Hz		= (uint8_t)0x04,
	GYRO_NOTCHBW_40Hz		= (uint8_t)0x05,
	GYRO_NOTCHBW_20Hz		= (uint8_t)0x06,
	GYRO_NOTCHBW_10Hz		= (uint8_t)0x07
}ICM42688_GyroNotch_t;


typedef enum{
	GYRO_FIRST_ORDER	= (uint8_t)0x00,
	GYRO_SECOND_ORDER	= (uint8_t)0x01,
	GYRO_THIRD_ORDER	= (uint8_t)0x02
}ICM42688_GyroUIFiltOrder_t;


typedef enum{
	GYRO_OFF		= (uint8_t)0x00,
	GYRO_STANDBY	= (uint8_t)0x01,
	GYRO_LOW_NOISE	= (uint8_t)0x03
}ICM42688_GyroMode_t;




/*
 * ============================================================================
 * 								ACCELOROMETER DEFINES
 * ============================================================================
 */
typedef enum{
	ACCEL_ODR_32KHz		= (uint8_t)0x01,
	ACCEL_ODR_16KHz		= (uint8_t)0x02,
	ACCEL_ODR_8KHz		= (uint8_t)0x03,
	ACCEL_ODR_4KHz		= (uint8_t)0x04,
	ACCEL_ODR_2KHz		= (uint8_t)0x05,
	ACCEL_ODR_1KHz		= (uint8_t)0x06,
	ACCEL_ODR_200Hz		= (uint8_t)0x07,
	ACCEL_ODR_100Hz		= (uint8_t)0x08,
	ACCEL_ODR_50Hz		= (uint8_t)0x09,
	ACCEL_ODR_25Hz		= (uint8_t)0x0A,
	ACCEL_ODR_12Hz5		= (uint8_t)0x0B,
	ACCEL_ODR_6Hz25		= (uint8_t)0x0C,
	ACCEL_ODR_3Hz125	= (uint8_t)0x0D,
	ACCEL_ODR_1Hz5625	= (uint8_t)0x0E,
	ACCEL_ODR_500Hz		= (uint8_t)0x0F
}ICM42688_AccelODR_t;


typedef enum{
	ACCEL_FSR_16g	= (uint8_t)0x00,
	ACCEL_FSR_8g	= (uint8_t)0x01,
	ACCEL_FSR_4g	= (uint8_t)0x02,
	ACCEL_FSR_2g	= (uint8_t)0x03
}ICM42688_AccelFSR_t;


typedef enum{
	ACCEL_FIRST_ORDER	= (uint8_t)0x00,
	ACCEL_SECOND_ORDER	= (uint8_t)0x01,
	ACCEL_THIRD_ORDER	= (uint8_t)0x02
}ICM42688_AccelUIFiltOrder_t;


typedef enum{
	ACCEL_OFF		= (uint8_t)0x00,
	ACCEL_LOW_POWER	= (uint8_t)0x02,
	ACCEL_LOW_NOISE	= (uint8_t)0x03
}ICM42688_AccelMode_t;




/*
 * ============================================================================
 * 							REGISTER BANK 0 ENUMS
 * ============================================================================
 */
/* Sensor Selection */
typedef enum{
	GYRO = 0,
	ACCEL = 1
}ICM42688_SensorSel_t;


/* DEVICE_CONFIG Defines */
typedef enum{
	SPI_MODE_0_3 = 0U,
	SPI_MODE_1_2 = 1U
}ICM42688_SPI_Mode_t;


/* DRIVE_CONFIG Defines */
typedef enum{
	SPI_SR_20NS_60NS	= 0x00U,
	SPI_SR_12NS_36NS	= 0x01U,
	SPI_SR_6NS_18NS		= 0x02U,
	SPI_SR_4NS_12NS		= 0x03U,
	SPI_SR_2NS_6NS		= 0x04U,
	SPI_SR_2NS			= 0x05U, //Less than 2ns
}ICM42688_SPI_SLEWRATE_t;


typedef enum{
	I2C_SR_20NS_60NS	= 0x00U,
	I2C_SR_12NS_36NS	= 0x01U,
	I2C_SR_6NS_18NS		= 0x02U,
	I2C_SR_4NS_12NS		= 0x03U,
	I2C_SR_2NS_6NS		= 0x04U,
	I2C_SR_2NS			= 0X05U	//Less than 2ns
}ICM42688_I2C_SLEWRATE_t;


/* INT_CONFIG Defines */
typedef enum{
	INT_ACTIVE_LOW 	= 0U,
	INT_ACTIVE_HIGH	= 1U,
	INT_POL_MAX		= 2U	//Just for sanity check
}ICM42688_Int_Polarity_t;


typedef enum{
	INT_OPEN_DRAIN	= 0U,
	INT_PUSH_PULL 	= 1U,
	INT_DRIVE_MAX	= 2U	//Just for sanity check
}ICM42688_Int_Drive_Circuit_t;


typedef enum{
	INT_PUSHED		= 0U,
	INT_LATCHED		= 1U,
	INT_MODE_MAX	= 2U	//Just for sanity check
}ICM42688_Int_Mode_t;


typedef enum{
	INT_AGC_RDY			= 0U,
	INT_FIFO_FULL 		= 1U,
	INT_FIFO_THS		= 2U,
	INT_DATA_RDY		= 3U,
	INT_RESET_DONE		= 4U,
	INT_PLL_RDY			= 5U,
	INT_UI_FSYNC		= 6U,
}ICM42688_Int_Status_t;


typedef enum{
	ICM42688_UI_SIFS_DISABLE_SPI	= 2U,
	ICM42688_UI_SIFS_DISABLE_I2C	= 3U,
}ICM42688_UI_SIFS_Cfg_t;


typedef enum{
	ICM42688_SENSOR_DATA_LITTLE_ENDIAN	= 0U,
	ICM42688_SENSOR_DATA_BIG_ENDIAN		= 1U,
}ICM42688_Sensor_Data_Endian_t;


typedef enum{
	ICM42688_FIFO_COUNT_LITTLE_ENDIAN	= 0U,
	ICM42688_FIFO_COUNT_BIG_ENDIAN		= 1U,
}ICM42688_FIFO_Count_Endian_t;


typedef enum{
	ICM42688_FIFO_COUNT_IN_BYTE		= 0U,
	ICM42688_FIFO_COUNT_IN_RECORD	= 1U,
}ICM42688_FIFO_Count_Rec_t;


typedef enum{
	ICM42688_FIFO_HOLD_LAST_DATA_INVALID	= 0U,
	ICM42688_FIFO_HOLD_LAST_DATA_VALID		= 1U,
}ICM42688_FIFO_Hold_Last_Data_En_t;


/* FIFO_CONFIG Defines */
typedef enum{
	BYPASS			= 0x00U,
	STREAM_TO_FIFO	= 0x01U,
	STOP_ON_FULL	= 0x02U
}ICM42688_FIFO_Mode_t;


/* REG_BANK_SEL Defines */
typedef enum{
	REG_BANK_0 		= 0x00U,
	REG_BANK_1 		= 0x01U,
	REG_BANK_2 		= 0x02U,
	REG_BANK_3 		= 0x03U,
	REG_BANK_4 		= 0x04U,
	UNKNOWN_BANK 	= 0x07U
}ICM42688_RegBank_t;


/* TEMPERATURE Enable/Disable */
typedef enum{
	TEMP_ENABLE		= 0x00U,
	TEMP_DISABLE	= 0x01U,
}ICM42688_Temp_t;


/* Temperature Filter Bandwidth */
typedef enum{
	TEMP_4000Hz_BW_0125ms_DLPF_LATENCY 	= 0x00U,
	TEMP_170Hz_BW_1ms_DLPF_LATENCY		= 0x01U,
	TEMP_82Hz_BW_2ms_DLPF_LATENCY		= 0x02U,
	TEMP_40Hz_BW_4ms_DLPF_LATENCY		= 0x03U,
	TEMP_20Hz_BW_8ms_DLPF_LATENCY		= 0x04U,
	TEMP_10Hz_BW_16ms_DLPF_LATENCY		= 0x05U,
	TEMP_5Hz_BW_32ms_DLPF_LATENCY		= 0x06U
}ICM42688_Temp_Filt_BW_t;


typedef enum{
	BW_ODR_DIV_2		= 0x00,
	BW_400Hz_ODR_DIV_4	= 0x01,
	BW_1x_AVG_FILT		= 0x01,
	BW_400Hz_ODR_DIV_5 	= 0x02,
	BW_400Hz_ODR_DIV_8	= 0x03,
	BW_400Hz_ODR_DIV_10	= 0x04,
	BW_400Hz_ODR_DIV_16	= 0x05,
	BW_400Hz_ODR_DIV_20	= 0x06,
	BW_16x_AVG_FILT		= 0x06,
	BW_400Hz_ODR_DIV_40	= 0x07,
	LOW_LATENCY_DEC2_400Hz_ODR = 0x0E,
	LOW_LATENCY_DEC2_200Hz_ODR = 0x0F,
}ICM42688_UIFilt_BW_t;


typedef enum{
	FIFO_GAT_DISABLE = 0x00, // GAT = Gyro Accel Temperature
	FIFO_GAT_ENABLE = 0x01,
}ICM42688_FIFO_GAT_En_t;


//HAVE TO CHECK THIS AGAIN!!!!!!!
typedef enum{
	FIFO_WM_GREATER_THS_ONESHOT	= 0x00,
	FIFO_WM_GREATER_THS_REPEAT	= 0x01,
}ICM42688_FIFO_WM_Mode_t;


typedef enum{
	FIFO_HIRES_DISABLE	= 0x00,	//FIFO stores normal (16bits) accel/gyro + temp
	FIFO_HIRES_ENABLE	= 0x01,	//FIFO stores extended: +3 bytes for an extended 20-bit accel/gyro + 1 byte temp
}ICM42688_FIFO_Hires_En_t;


typedef enum{
	FIFO_PARTIAL_READ_DISABLE	= 0x00,	//Partial FIFO read disable: must re-read the entire FIFO
	FIFO_PARTIAL_READ_ENABLE	= 0x01,	//Partial FIFO read enable: resume from the last read point
}ICM42688_FIFO_Resume_Read_t;




/*
 * ===============================================================================
 * 		   STRUCT HOLDS EVERY DEFINED/CACHED/DEFAULT VALUES OF ICM42688P
 * ===============================================================================
 */
typedef struct{
	SPI_HandleTypeDef		*hspi;
	GPIO_TypeDef			*cs_port;
	uint16_t				cs_pin;

	ICM42688_SPI_Mode_t		spi_mode;
	ICM42688_SPI_SLEWRATE_t	spi_slew_rate;
}ICM42688_SPI_Config_t;


typedef struct{
	ICM42688_GyroODR_t 				gyro_odr;
	ICM42688_GyroFSR_t				gyro_fsr;
	ICM42688_GyroNotch_t			gyro_notch;
	ICM42688_GyroUIFiltOrder_t		gyro_filt_order;
	ICM42688_GyroMode_t				gyro_mode;
	ICM42688_UIFilt_BW_t			gyro_uifilt_bw;
}ICM42688_Gyro_Config_t;


typedef struct{
	ICM42688_AccelODR_t				accel_odr;
	ICM42688_AccelFSR_t				accel_fsr;
	ICM42688_AccelUIFiltOrder_t		accel_filt_order;
	ICM42688_AccelMode_t			accel_mode;
	ICM42688_UIFilt_BW_t			accel_uifilt_bw;
}ICM42688_Accel_Config_t;


typedef struct{
	ICM42688_Int_Polarity_t			int1_polarity;
	ICM42688_Int_Drive_Circuit_t	int1_drive;
	ICM42688_Int_Mode_t				int1_mode;
}ICM42688_Int1_Config_t;


typedef struct{
	ICM42688_Int_Polarity_t			int2_polarity;
	ICM42688_Int_Drive_Circuit_t	int2_drive;
	ICM42688_Int_Mode_t				int2_mode;
}ICM42688_Int2_Config_t;


typedef struct{
	ICM42688_UI_SIFS_Cfg_t				ui_sifs_config;
	ICM42688_Sensor_Data_Endian_t		sensor_data_endian;
	ICM42688_FIFO_Count_Endian_t		fifo_count_endian;
	ICM42688_FIFO_Count_Rec_t			fifo_count_rec;
	ICM42688_FIFO_Hold_Last_Data_En_t	fifo_hold_last_data;
}ICM42688_Intf_Config0_t;


typedef struct{
	ICM42688_Temp_t		temp_state;
}ICM42688_Temp_Config_t;


typedef struct{
	ICM42688_FIFO_Mode_t			fifo_mode;
	uint16_t						fifo_watermark;

	ICM42688_FIFO_GAT_En_t			fifo_gyro_state;
	ICM42688_FIFO_GAT_En_t			fifo_accel_state;
	ICM42688_FIFO_GAT_En_t			fifo_temp_state;

	ICM42688_FIFO_WM_Mode_t			fifo_wm_mode;
	ICM42688_FIFO_Hires_En_t		fifo_hires_state;
	ICM42688_FIFO_Resume_Read_t		fifo_partial_read_state;
}ICM42688_FIFO_Config_t;


typedef struct{
	uint8_t	int_status;
}ICM42688_Cached_Val_t;



/* Struct stores every important thing */
typedef struct{
	float 	gyro_dps_per_lsb;
	float	gyro_lsb_per_dps_dtsheet;
	float 	accel_g_per_lsb;
	float	accel_lsb_per_g_dtsheet;

	ICM42688_SPI_Config_t		spi_config;
	ICM42688_Gyro_Config_t		gyro_config;
	ICM42688_Accel_Config_t		accel_config;
	ICM42688_Int1_Config_t		int1_config;
	ICM42688_Int2_Config_t		int2_config;
	ICM42688_Intf_Config0_t		intf_config;
	ICM42688_Temp_Config_t		temp_config;
	ICM42688_FIFO_Config_t 		fifo_config;
	ICM42688_Cached_Val_t		cached;

	bool is_initialized;
	bool is_reset;
	bool is_icm42688_alive;
}ICM42688_Handle_t;

#include "imu/icm42688_rw.h"

#endif /* INC_ICM42688_H_ */
