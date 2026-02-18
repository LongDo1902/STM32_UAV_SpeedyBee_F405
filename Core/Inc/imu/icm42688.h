/*
 * icm42688.h
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
#include "stm32f4xx_hal.h"
#include "spi.h"


/*
 * =============================================================
 * 						ICM42688-P MACROS
 * =============================================================
 */
#ifndef	ICM42688_WRITE_READ_WITH_BANKED
#define ICM42688_WRITE_READ_WITH_BANKED		1
#endif


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
#define ICM42688_RAW_INVALID			((int16_t)INT16_MIN)



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

/* FIFO_CONFIG Defines */
typedef enum{
	BYPASS			= 0x00U,
	STREAM_TO_FIFO	= 0x01U,
	STOP_ON_FULL	= 0x02U
}ICM42688_FIFO_MODE_t;

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
}ICM42688_Gyro_Config_t;

typedef struct{
	ICM42688_AccelODR_t				accel_odr;
	ICM42688_AccelFSR_t				accel_fsr;
	ICM42688_AccelUIFiltOrder_t		accel_filt_order;
	ICM42688_AccelMode_t			accel_mode;
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
	ICM42688_Temp_t		temp_state;
}ICM42688_Temp_Config_t;

typedef struct{
	/* Gyro and Accel config */
	ICM42688_Gyro_Config_t	gyro_config;
	ICM42688_Accel_Config_t	accel_config;

	/* SPI config */
	ICM42688_SPI_Config_t	spi_config;

	/* Interrupt config */
	ICM42688_Int1_Config_t	int1_config;
	ICM42688_Int2_Config_t	int2_config;

	/* Other runtime setups */
	float 	gyro_dps_per_lsb;	//Scale factor: raw gyro LSB -> dps
	float	gyro_lsb_per_dps_dtsheet;

	float 	accel_g_per_lsb;	//Scale factor: raw accel LSB -> g
	float	accel_lsb_per_g_dtsheet;

	/* Flags/variable to check and reference parameters */
	bool is_initialized;
	bool is_reset;
	bool is_alive;

	/* Temperature settings */
	ICM42688_Temp_Config_t temp_config;
}ICM42688_Handle_t;




/*
 * ===================================================================================
 *							ICM42688 MASKS
 * ===================================================================================
 */
/* Generic Helpers */
#define ICM42688_BIT(pos)							(1U << (pos))
#define	ICM42688_FIELD_MSK(pos, width)				(((1U << (width)) - 1U) << (pos))
#define ICM42688_FIELD_VAL(val, pos, msk)			((uint8_t)((val << pos) & msk))

typedef uint16_t ICM42688_Reg_t;
#define ICM42688_REG(bank, addr)	((ICM42688_Reg_t)((((uint16_t)(bank)) << 8) | ((uint8_t)(addr))))
#define ICM42688_REG_BANK(r)		((ICM42688_RegBank_t)(((r) >> 8) & 0xFF))
#define ICM42688_REG_ADDR(r)		((uint8_t)((r) & 0xFF))

/* DEVICE_CONFIG Fields */
#define ICM42688_DEVICE_CONFIG_SOFT_RESET_Pos		0U
#define ICM42688_DEVICE_CONFIG_SOFT_RESET_Msk		ICM42688_BIT(ICM42688_DEVICE_CONFIG_SOFT_RESET_Pos)

#define	ICM42688_DEVICE_CONFIG_SPI_MODE_Pos			4U
#define	ICM42688_DEVICE_CONFIG_SPI_MODE_Msk			ICM42688_BIT(ICM42688_DEVICE_CONFIG_SPI_MODE_Pos)
#define ICM42688_DEVICE_CONFIG_SPI_MODE_Val(val)	ICM42688_FIELD_VAL((val), ICM42688_DEVICE_CONFIG_SPI_MODE_Pos, ICM42688_DEVICE_CONFIG_SPI_MODE_Msk)

/* DRIVE_CONFIG Fields */
#define ICM42688_DRIVE_CONFIG_SPI_SR_Pos			0U
#define ICM42688_DRIVE_CONFIG_SPI_SR_Msk			ICM42688_FIELD_MSK(ICM42688_DRIVE_CONFIG_SPI_SR_Pos, 3U)
#define ICM42688_DRIVE_CONFIG_SPI_SR_Val(val)		ICM42688_FIELD_VAL(val, ICM42688_DRIVE_CONFIG_SPI_SR_Pos, ICM42688_DRIVE_CONFIG_SPI_SR_Msk)

#define ICM42688_DRIVE_CONFIG_I2C_SR_Pos			3U
#define ICM42688_DRIVE_CONFIG_I2C_SR_Msk			ICM42688_FIELD_MSK(ICM42688_DRIVE_CONFIG_I2C_SR_Pos, 3U)
#define ICM42688_DRIVE_CONFIG_I2C_SR_Val(val)		ICM42688_FIELD_VAL(val, ICM42688_DRIVE_CONFIG_I2C_SR_Pos, ICM42688_DRIVE_CONFIG_I2C_SR_Msk)

/* INT_CONFIG Fields */
#define ICM42688_INT1_POL_Pos			0U
#define ICM42688_INT1_POL_Msk			ICM42688_BIT(ICM42688_INT1_POL_Pos)
#define ICM42688_INT1_POL_Val(val)		ICM42688_FIELD_VAL(val, ICM42688_INT1_POL_Pos, ICM42688_INT1_POL_Msk)

#define ICM42688_INT1_DRIVE_Pos			1U
#define ICM42688_INT1_DRIVE_Msk			ICM42688_BIT(ICM42688_INT1_DRIVE_Pos)
#define ICM42688_INT1_DRIVE_Val(val)	ICM42688_FIELD_VAL(val, ICM42688_INT1_DRIVE_Pos, ICM42688_INT1_DRIVE_Msk)

#define ICM42688_INT1_MODE_Pos			2U
#define ICM42688_INT1_MODE_Msk			ICM42688_BIT(ICM42688_INT1_MODE_Pos)
#define ICM42688_INT1_MODE_Val(val)		ICM42688_FIELD_VAL(val, ICM42688_INT1_MODE_Pos, ICM42688_INT1_MODE_Msk)

#define ICM42688_INT2_POL_Pos			3U
#define ICM42688_INT2_POL_Msk			ICM42688_BIT(ICM42688_INT2_POL_Pos)
#define ICM42688_INT2_POL_Val(val)		ICM42688_FIELD_VAL(val, ICM42688_INT2_POL_Pos, ICM42688_INT2_POL_Msk)

#define ICM42688_INT2_DRIVE_Pos			4U
#define ICM42688_INT2_DRIVE_Msk			ICM42688_BIT(ICM42688_INT2_DRIVE_Pos)
#define ICM42688_INT2_DRIVE_Val(val)	ICM42688_FIELD_VAL(val, ICM42688_INT2_DRIVE_Pos, ICM42688_INT2_DRIVE_Msk)

#define ICM42688_INT2_MODE_Pos			5U
#define ICM42688_INT2_MODE_Msk			ICM42688_BIT(ICM42688_INT2_MODE_Pos)
#define ICM42688_INT2_MODE_Val(val)		ICM42688_FIELD_VAL(val, ICM42688_INT2_MODE_Pos, ICM42688_INT2_MODE_Msk)


/* GYRO_CONFIG0 fields */
#define ICM42688_GYRO_ODR_Pos			0U
#define ICM42688_GYRO_ODR_Msk			ICM42688_FIELD_MSK(ICM42688_GYRO_ODR_Pos, 4U)
#define ICM42688_GYRO_ODR_Val(val)		ICM42688_FIELD_VAL(val, ICM42688_GYRO_ODR_Pos, ICM42688_GYRO_ODR_Msk)

#define ICM42688_GYRO_FS_SEL_Pos		5U
#define ICM42688_GYRO_FS_SEL_Msk		ICM42688_FIELD_MSK(ICM42688_GYRO_FS_SEL_Pos, 3U)
#define ICM42688_GYRO_FS_SEL_Val(val)	ICM42688_FIELD_VAL(val, ICM42688_GYRO_FS_SEL_Pos, ICM42688_GYRO_FS_SEL_Msk)


/* ACCEL_CONFIG0 fields */
#define ICM42688_ACCEL_ODR_Pos			0U
#define ICM42688_ACCEL_ODR_Msk			ICM42688_FIELD_MSK(ICM42688_ACCEL_ODR_Pos, 4U)
#define ICM42688_ACCEL_ODR_Val(val) 	ICM42688_FIELD_VAL(val, ICM42688_ACCEL_ODR_Pos, ICM42688_ACCEL_ODR_Msk)

#define ICM42688_ACCEL_FS_SEL_Pos		5U
#define ICM42688_ACCEL_FS_SEL_Msk		ICM42688_FIELD_MSK(ICM42688_ACCEL_FS_SEL_Pos, 3U)
#define ICM42688_ACCEL_FS_SEL_Val(val)	ICM42688_FIELD_VAL(val, ICM42688_ACCEL_FS_SEL_Pos, ICM42688_ACCEL_FS_SEL_Msk)


/* PWR_MGMT0 fields */
#define ICM42688_ACCEL_MODE_Pos			0U
#define ICM42688_ACCEL_MODE_Msk			ICM42688_FIELD_MSK(ICM42688_ACCEL_MODE_Pos, 2U)
#define ICM42688_ACCEL_MODE_Val(val)	ICM42688_FIELD_VAL(val, ICM42688_ACCEL_MODE_Pos, ICM42688_ACCEL_MODE_Msk)

#define ICM42688_GYRO_MODE_Pos			2U
#define ICM42688_GYRO_MODE_Msk			ICM42688_FIELD_MSK(ICM42688_GYRO_MODE_Pos, 2U)
#define ICM42688_GYRO_MODE_Val(val)		ICM42688_FIELD_VAL(val, ICM42688_GYRO_MODE_Pos, ICM42688_GYRO_MODE_Msk)

#define ICM42688_IDLE_Pos				4U
#define ICM42688_IDLE_Msk				ICM42688_BIT(ICM42688_IDLE_Pos)
#define ICM42688_IDLE_Val(val)			ICM42688_FIELD_VAL(val, ICM42688_IDLE_Pos, ICM42688_IDLE_Msk)

#define ICM42688_TEMP_Pos				5U
#define ICM42688_TEMP_Msk				ICM42688_BIT(ICM42688_TEMP_Pos)
#define ICM42688_TEMP_Val(val)			ICM42688_FIELD_VAL(val, ICM42688_TEMP_Pos, ICM42688_TEMP_Msk)


/* SENSOR_CONFIG0 fields */
#define ICM42688_XA_DISABLE_Pos		0U
#define ICM42688_YA_DISABLE_Pos		1U
#define ICM42688_ZA_DISABLE_Pos		2U
#define ICM42688_XG_DISABLE_Pos		3U
#define	ICM42688_YG_DISABLE_Pos		4U
#define ICM42688_ZG_DISABLE_Pos		5U


/* GYRO_CONFIG1 fields */
#define ICM42688_GYRO_DEC2_M2_ORD_Pos	0U
#define ICM42688_GYRO_UI_FILT_ORD_Pos	2U
#define ICM42688_TEMP_FILT_BW_Pos



/*
 * =============================================================
 * 							PUBLIC APIs
 * =============================================================
 */
#if ICM42688_WRITE_READ_WITH_BANKED
	HAL_StatusTypeDef ICM42688_WriteReg(ICM42688_Handle_t *handle, ICM42688_Reg_t reg, uint8_t val);
	HAL_StatusTypeDef ICM42688_ReadReg(ICM42688_Handle_t *handle, ICM42688_Reg_t reg, uint8_t* outval);
	HAL_StatusTypeDef ICM42688_ReadRegs(ICM42688_Handle_t* handle, ICM42688_Reg_t startRegAddr, uint8_t* buf, uint16_t bufLength);
#else
	HAL_StatusTypeDef ICM42688_WriteReg(ICM42688_Handle_t *handle, uint8_t regAddr, uint8_t val);
	HAL_StatusTypeDef ICM42688_ReadReg(ICM42688_Handle_t* handle, uint8_t regAddr, uint8_t* outVal);
	HAL_StatusTypeDef ICM42688_ReadRegs(ICM42688_Handle_t* handle, uint8_t startRegAddr, uint8_t* buf, uint16_t bufLength);
#endif

HAL_StatusTypeDef ICM42688_Set_RegBank(ICM42688_Handle_t* handle, ICM42688_RegBank_t regBank);
HAL_StatusTypeDef ICM42688_Get_RegBankSensor(ICM42688_Handle_t* handle, ICM42688_RegBank_t* regBank);
HAL_StatusTypeDef ICM42688_IsAlive(ICM42688_Handle_t* handle);
HAL_StatusTypeDef ICM42688_SoftReset(ICM42688_Handle_t* handle);

HAL_StatusTypeDef ICM42688_Set_SPI_Mode(ICM42688_Handle_t* handle, ICM42688_SPI_Mode_t spiMode);
HAL_StatusTypeDef ICM42688_Get_SPI_SlewRate(ICM42688_Handle_t* handle, ICM42688_SPI_SLEWRATE_t* slewRate);
HAL_StatusTypeDef ICM42688_Set_SPI_SlewRate(ICM42688_Handle_t* handle, ICM42688_SPI_SLEWRATE_t slewRate);


HAL_StatusTypeDef ICM42688_Set_GyroMode(ICM42688_Handle_t* handle, ICM42688_GyroMode_t mode);
HAL_StatusTypeDef ICM42688_Set_GyroODR(ICM42688_Handle_t* handle, ICM42688_GyroODR_t odr);
HAL_StatusTypeDef ICM42688_Set_GyroFS(ICM42688_Handle_t* handle, ICM42688_GyroFSR_t fullScale);
HAL_StatusTypeDef ICM42688_Set_GyroConfig(ICM42688_Handle_t* handle,
										  ICM42688_GyroMode_t mode,
										  ICM42688_GyroODR_t odr,
										  ICM42688_GyroFSR_t fsr);

HAL_StatusTypeDef ICM42688_Set_AccelMode(ICM42688_Handle_t* handle, ICM42688_AccelMode_t mode);
HAL_StatusTypeDef ICM42688_Set_AccelODR(ICM42688_Handle_t* handle, ICM42688_AccelODR_t odr);
HAL_StatusTypeDef ICM42688_Set_AccelFS(ICM42688_Handle_t* handle, ICM42688_AccelFSR_t fullScale);
HAL_StatusTypeDef ICM42688_Set_AccelConfig(ICM42688_Handle_t* handle,
										   ICM42688_AccelMode_t mode,
										   ICM42688_AccelODR_t odr,
										   ICM42688_AccelFSR_t fsr);

HAL_StatusTypeDef ICM42688_Set_Int1_Config(ICM42688_Handle_t* handle,
										   ICM42688_Int_Polarity_t polarity,
										   ICM42688_Int_Drive_Circuit_t drive,
										   ICM42688_Int_Mode_t mode);

HAL_StatusTypeDef ICM42688_Set_Int2_Config(ICM42688_Handle_t* handle,
										   ICM42688_Int_Polarity_t polarity,
										   ICM42688_Int_Drive_Circuit_t drive,
										   ICM42688_Int_Mode_t mode);


#endif /* INC_ICM42688_H_ */








