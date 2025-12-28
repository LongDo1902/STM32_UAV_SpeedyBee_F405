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

/*
 * =============================================================
 * 						ICM42688-P Macros
 * =============================================================
 */
#define ICM42688_VDD				1800U	//In mVs

/* Frequencies are declared in Hz */
#define ICM42688_SPI_MAX_CLKFREQ	24000000U
#define ICM42688_I2C_MAX_CLKFREQ	1000000U
#define ICM42688_MAX_RTC_FREQ		50000U
#define ICM42688_TYP_RTC_FREQ		32000U
#define ICM42688_MIN_RTC_FREQ		31000U


/*
 * ============================================================================
 * 								Gyroscope Defines
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
	GYRO_FSR_16dps625	= (uint8_t)0x07
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


/*
 * ============================================================================
 * 								Accelerometer Defines
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


/*
 * ===============================================================================
 * 				STRUCT HOLDS EVERY DEFINED CHARACTERISTICS OF ICM42688P
 * ===============================================================================
 */
typedef struct{
	ICM42688_GyroODR_t 			gyro_odr;
	ICM42688_GyroFSR_t			gyro_fsr;
	ICM42688_GyroNotch_t		gyro_noth;
	ICM42688_GyroUIFiltOrder_t	gyro_filt_order;

	ICM42688_AccelODR_t			accel_odr;
	ICM42688_AccelFSR_t			accel_fsr;
	ICM42688_AccelUIFiltOrder_t	accel_filt_order;
}ICM42688_Config_t;

typedef struct{
	ICM42688_Config_t	config;	//Stores "How is the sensor setup?"
	float gyro_lsb_to_dps;		//Stores "What is the math multiplier for this setup?"
	float accel_lsb_to_dps;		//Stores...
}ICM42688_Handle_t;


#endif /* INC_ICM42688_H_ */








