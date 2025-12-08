/*
 * target.h
 *
 *  Created on: Nov 13, 2025
 *      Author: dobao
 */

#ifndef INC_TARGET_H_
#define INC_TARGET_H_

/*
 * ==============================================================
 * 						LED PIN CONFIGURATIONS
 * ==============================================================
 */
#define LED1_BLUE_PIN		PC13

/*
 * ==============================================================
 * 						IMU PIN CONFIGURATIONS
 * ==============================================================
 */
#define ICM42688P_SCK_PIN	PA5
#define ICM42688P_MISO_PIN	PA6
#define ICM42688P_MOSI_PIN	PA7
#define ICM42688p_CS_PIN	PA4

/*
 * ==============================================================
 * 						BARO PIN CONFIGURATIONS
 * ==============================================================
 */
#define DPS310_SCL_PIN	PB8
#define DPS310_SDA_PIN	PB9

#endif /* INC_TARGET_H_ */
