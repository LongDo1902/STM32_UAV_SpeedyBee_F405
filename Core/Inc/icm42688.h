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
#define ICM42688_SPI_TIMEOUT_MS				10U		//10ms
#define ICM42688_SPI_ADDR_MASK				0x7FU
#define ICM42688_SPI_READ_BIT				0x80U
#define ICM42688_WHO_AM_I_DEFAULT			0x47U


typedef enum{
	ICM42688_ERROR,
	ICM42688_OK,
	ICM42688_TIMEOUT
}ICM42688_Status_t;

typedef enum{
	REG_BANK_0 = 0x00U,
	REG_BANK_1 = 0x01U,
	REG_BANK_2 = 0x02U,
	REG_BANK_3 = 0x03U,
	REG_BANK_4 = 0x04U
}ICM42688_RegBank_t;


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


/*
 * ===============================================================================
 * 				STRUCT HOLDS EVERY DEFINED CHARACTERISTICS OF ICM42688P
 * ===============================================================================
 */
typedef struct{
	ICM42688_GyroODR_t 			gyro_odr;
	ICM42688_GyroFSR_t			gyro_fsr;
	ICM42688_GyroNotch_t		gyro_notch;
	ICM42688_GyroUIFiltOrder_t	gyro_filt_order;

	ICM42688_AccelODR_t			accel_odr;
	ICM42688_AccelFSR_t			accel_fsr;
	ICM42688_AccelUIFiltOrder_t	accel_filt_order;
}ICM42688_Config_t;

typedef struct{
	float 				gyro_lsb_to_dps;	//Stores "What is the math multiplier for this setup?"
	float 				accel_lsb_to_g;		//Stores...

	SPI_HandleTypeDef 	*hspi;
	GPIO_TypeDef 		*cs_port;
	uint16_t			cs_pin;

	ICM42688_RegBank_t	regBank;
}ICM42688_Handle_t;


/*
 * ===================================================================================
 *							ICM42688 MASKS
 * ===================================================================================
 */
#define ICM42688_DEV_CONF_SOFT_RESET_Pos		0U
#define ICM42688_DEV_CONF_SOFT_RESET_Msk		(1U << ICM42688_SOFT_RESET_Pos)



/*
 * ===================================================================================
 *							ICM42688 REGISTER MAP (UB0)
 * ===================================================================================
 */
/* Configuration & Setup */
#define ICM42688_UB0_DEVICE_CONF		0x11U
#define ICM42688_UB0_DRIVE_CONF			0x13U
#define ICM42688_UB0_INT_CONF			0x14U
#define ICM42688_UB0_FIFO_CONF			0x16U
#define ICM42688_UB0_SIGNAL_PATH_RST	0x4BU
#define ICM42688_UB0_INTF_CONF0			0x4CU
#define ICM42688_UB0_INTF_CONF1			0x4DU
#define ICM42688_UB0_PWR_MGMT0			0x4EU

/* Interrupt & FIFO Status */
#define ICM42688_UB0_INT_STATUS			0x2DU
#define ICM42688_UB0_INT_STATUS2		0x37U
#define ICM42688_UB0_INT_STATUS3		0x38U
#define ICM42688_UB0_FIFO_COUNTH		0x2EU
#define ICM42688_UB0_FIFO_COUNTL		0x2FU
#define ICM42688_UB0_FIFO_DATA			0x30U

/* Sensor Configuration */
#define ICM42688_GYRO_CONF0				0x4FU
#define ICM42688_GYRO_CONF1				0x51U
#define ICM42688_ACCEL_CONF0			0x50U
#define ICM42688_ACCEL_CONF1			0x53U
#define ICM42688_UB0_GYRO_ACCEL_CONF0	0x52U


/* APEX & Features */
#define ICM42688_UB0_APEX_DATA0			0x31U
#define ICM42688_UB0_APEX_DATA1			0x32U
#define ICM42688_UB0_APEX_DATA2			0x33U
#define ICM42688_UB0_APEX_DATA3			0x34U
#define ICM42688_UB0_APEX_DATA4			0x35U
#define ICM42688_UB0_APEX_DATA5			0x36U
#define ICM42688_UB0_APEX_CONF			0x56U
#define ICM42688_UB0_SMD_CONF			0x57U

/* Advanced Controls */
#define ICM42688_UB0_TMST_FSYNCH		0x2BU
#define ICM42688_UB0_TMST_FSYNCL		0x2CU
#define ICM42688_UB0_TMST_CONF			0x54U
#define ICM42688_UB0_FIFO_CONF1			0x5FU
#define ICM42688_UB0_FIFO_CONF2			0x60U
#define ICM42688_UB0_FIFO_CONF3			0x61U
#define ICM42688_UB0_FSYNC_CONF			0x62U

/* Interrupt Sources */
#define ICM42688_UB0_INT_CONF0			0x63U
#define ICM42688_UB0_INT_CONF1			0x64U
#define ICM42688_UB0_INT_SRC0			0x65U
#define ICM42688_UB0_INT_SRC1			0x66U
#define ICM42688_UB0_INT_SRC3			0x68U
#define ICM42688_UB0_INT_SRC4			0x69U

/* System */
#define ICM42688_UB0_FIFO_LOST_PKT0		0x6CU
#define ICM42688_UB0_FIFO_LOST_PKT1		0x6DU
#define ICM42688_UB0_SELFTEST_CONF		0x70U
#define ICM42688_UB0_WHO_AM_I			0x75U
#define ICM42688_UB0_REG_BANK_SEL		0x76U

/* Sensor Data (High / Low byte pairs) */
#define ICM42688_UB0_TEMP_DATA1			0x1DU
#define ICM42688_UB0_TEMP_DATA0			0x1EU

#define ICM42688_UB0_ACCEL_DATA_X1		0x1FU
#define ICM42688_UB0_ACCEL_DATA_X0		0x20U
#define ICM42688_UB0_ACCEL_DATA_Y1		0x21U
#define ICM42688_UB0_ACCEL_DATA_Y0		0x22U
#define ICM42688_UB0_ACCEL_DATA_Z1		0x23U
#define ICM42688_UB0_ACCEL_DATA_Z0		0x24U

#define ICM42688_UB0_GYRO_DATA_X1		0x25U
#define ICM42688_UB0_GYRO_DATA_X0		0x26U
#define ICM42688_UB0_GYRO_DATA_Y1		0x27U
#define ICM42688_UB0_GYRO_DATA_Y0		0x28U
#define ICM42688_UB0_GYRO_DATA_Z1		0x29U
#define ICM42688_UB0_GYRO_DATA_Z0		0x2AU


/*
 * ===================================================================================
 *							ICM42688 REGISTER MAP (UB1)
 * ===================================================================================
 */
#define ICM42688_UB1_SENSOR_CONF0		0x03U
#define ICM42688_UB1_GYRO_CONF_STATIC2	0x0BU
#define ICM42688_UB1_GYRO_CONF_STATIC3	0x0CU
#define ICM42688_UB1_GYRO_CONF_STATIC4	0x0DU
#define ICM42688_UB1_GYRO_CONF_STATIC5	0x0EU
#define ICM42688_UB1_GYRO_CONF_STATIC6	0x0FU
#define ICM42688_UB1_GYRO_CONF_STATIC7	0x10U
#define ICM42688_UB1_GYRO_CONF_STATIC8	0x11U
#define ICM42688_UB1_GYRO_CONF_STATIC9	0x12U
#define ICM42688_UB1_GYRO_CONF_STATIC10	0x13U

#define ICM42688_UB1_XG_ST_DATA			0x5FU
#define ICM42688_UB1_YG_ST_DATA			0x60U
#define ICM42688_UB1_ZG_ST_DATA			0x61U
#define ICM42688_UB1_TMST_VAL0			0x62U
#define ICM42688_UB1_TMST_VAL1			0x63U
#define ICM42688_UB1_TMST_VAL2			0x64U

#define ICM42688_UB1_INTF_CONF4			0x7AU
#define ICM42688_UB1_INTF_CONF5			0x7BU
#define ICM42688_UB1_INTF_CONF6			0x7CU


/*
 * ===================================================================================
 *							ICM42688 REGISTER MAP (UB2)
 * ===================================================================================
 */
#define ICM42688_UB2_ACCEL_CONF_STATIC2	0x03U
#define ICM42688_UB2_ACCEL_CONF_STATIC3	0x04U
#define ICM42688_UB2_ACCEL_CONF_STATIC4	0x05U
#define ICM42688_UB2_XA_ST_DATA			0x3BU
#define ICM42688_UB2_YA_ST_DATA			0x3CU
#define ICM42688_UB2_ZA_ST_DATA			0x3DU


/*
 * ===================================================================================
 *							ICM42688 REGISTER MAP (UB4)
 * ===================================================================================
 */
#define ICM42688_UB4_APEX_CONF1			0x40U
#define ICM42688_UB4_APEX_CONF2			0x41U
#define ICM42688_UB4_APEX_CONF3			0x42U
#define ICM42688_UB4_APEX_CONF4			0x43U
#define ICM42688_UB4_APEX_CONF5			0x44U
#define ICM42688_UB4_APEX_CONF6			0x45U
#define ICM42688_UB4_APEX_CONF7			0x46U
#define ICM42688_UB4_APEX_CONF8			0x47U
#define ICM42688_UB4_APEX_CONF9			0x48U

#define ICM42688_UB4_ACCEL_WOM_X_THR	0x4AU
#define ICM42688_UB4_ACCEL_WOM_Y_THR	0x4BU
#define ICM42688_UB4_ACCEL_WOM_Z_THR	0x4CU
#define ICM42688_UB4_INT_SRC6			0x4DU
#define ICM42688_UB4_INT_SRC7			0x4EU
#define ICM42688_UB4_INT_SRC8			0x4FU
#define ICM42688_UB4_INT_SRC9			0x50U
#define ICM42688_UB4_INT_SRC10			0x51U

#define	ICM42688_UB4_OFFSET_USER0		0x77U
#define ICM42688_UB4_OFFSET_USER1		0x78U
#define ICM42688_UB4_OFFSET_USER2		0x79U
#define ICM42688_UB4_OFFSET_USER3		0x7AU
#define ICM42688_UB4_OFFSET_USER4		0x7BU
#define ICM42688_UB4_OFFSET_USER5		0x7CU
#define ICM42688_UB4_OFFSET_USER6		0x7DU
#define ICM42688_UB4_OFFSET_USER7		0x7EU
#define ICM42688_UB4_OFFSET_USER8		0x7FU


/*
 * =============================================================
 * 							PUBLIC APIs
 * =============================================================
 */
HAL_StatusTypeDef ICM42688_WriteReg(ICM42688_Handle_t *handle, uint8_t regAddr, uint8_t val);
HAL_StatusTypeDef ICM42688_ReadReg(ICM42688_Handle_t* handle, uint8_t regAddr, uint8_t* outVal);
HAL_StatusTypeDef ICM42688_ReadRegs(ICM42688_Handle_t* handle, uint8_t startRegAddr, uint8_t* buf, uint16_t bufLength);

HAL_StatusTypeDef ICM42688_IsAlive(ICM42688_Handle_t* handle);
void ICM42688_Init(ICM42688_Handle_t* handle, ICM42688_Config_t config);

#endif /* INC_ICM42688_H_ */








