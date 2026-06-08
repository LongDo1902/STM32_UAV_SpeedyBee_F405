/*
 * icm42688_lpf.h
 *
 * Software low-pass filter for ICM42688 scaled IMU data
 */

#ifndef ICM42688_LPF_H
#define ICM42688_LPF_H

#include <stdbool.h>
#include <stdint.h>

#include "imu/sensors/icm42688_data.h"

/**
 * @brief   One low-pass filter object for one axis of accel or gyroscope
 */
typedef struct
{
    float prev_filtered_val;
    float cutoff_hz;
    bool  is_initialized;
} ICM42688_PT1_Filter_t;


/**
 * @brief   IMU low-pass filter object for the whole IMU
 */
typedef struct
{
    ICM42688_PT1_Filter_t accel_lpf[3];
    ICM42688_PT1_Filter_t gyro_lpf[3];

    float accel_cutoff_hz;
    float gyro_cutoff_hz;

    float nominal_dt_s;
    bool  is_initialized;
} ICM42688_IMU_LPF_t;



/**
 * @brief   Initialize the IMU low-pass filter
 */
bool ICM42688_IMU_LPF_Init(ICM42688_IMU_LPF_t *filter, float nominal_dt_s, float gyro_cutoff_hz, float accel_cutoff_hz);



/**
 * @brief   Reset the IMU low-pass filter
 */
bool ICM42688_IMU_LPF_Reset(ICM42688_IMU_LPF_t *filter);



/**
 * @brief
 */
bool ICM42688_IMU_LPF_Update(ICM42688_IMU_LPF_t *filter, const ICM42688_Temp_Accel_Gyro_Scaled_t *input, ICM42688_Temp_Accel_Gyro_Scaled_t *output);



/**
 * @brief
 */
bool ICM42688_IMU_LPF_Update_Dt(ICM42688_IMU_LPF_t *filter, const ICM42688_Temp_Accel_Gyro_Scaled_t *input, ICM42688_Temp_Accel_Gyro_Scaled_t *output, float dt_s);

#endif /* ICM42688_LPF_H */