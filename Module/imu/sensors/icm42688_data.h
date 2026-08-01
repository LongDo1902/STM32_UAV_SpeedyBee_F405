/*
 * icm42688_data.h
 *
 *  Created on: Mar 14, 2026
 *      Author: dobaolong
 */

#ifndef INC_IMU_SENSORS_ICM42688_DATA_H_
#define INC_IMU_SENSORS_ICM42688_DATA_H_

#include "imu/core/icm42688_masks.h"
#include "imu/core/icm42688_registers.h"
#include "imu/core/icm42688_rw.h"
#include "imu/core/icm42688_types.h"

typedef struct
{
    int16_t raw_temperature;
    int16_t raw_accel[3];
    int16_t raw_gyro[3];

} ICM42688_Raw_t;


typedef struct
{
    int32_t offset_raw_accel[3];
    int32_t offset_raw_gyro[3];

} ICM42688_Offset_Raw_t;


typedef enum
{
    // IMU Z+ aligned with body Z+.
    IMU_ORIENT_X_Y_Z,
    IMU_ORIENT_NEGY_NEGX_Z,
    IMU_ORIENT_Y_NEG_X_Z,
    IMU_ORIENT_NEGX_NEGY_Z,
    IMU_ORIENT_NEGY_X_Z,

    // IMU upside down: IMU Z- aligned with body Z+.
    IMU_ORIENT_X_NEGY_NEGZ,
    IMU_ORIENT_NEGY_NEGX_NEGZ,
    IMU_ORIENT_NEGX_Y_NEGZ,
    IMU_ORIENT_Y_X_NEGZ,

    IMU_ORIENT_COUNT,
} ICM42688_Orientation_t;


typedef enum
{
    AXIS_X,
    AXIS_NEG_X,

    AXIS_Y,
    AXIS_NEG_Y,

    AXIS_Z,
    AXIS_NEG_Z
} ICM42688_Axis_t;


typedef struct
{
    ICM42688_Axis_t body_x;
    ICM42688_Axis_t body_y;
    ICM42688_Axis_t body_z;
} ICM42688_Remap_Axes_t;


typedef struct
{
    float roll;
    float pitch;
    float yaw;
} ICM42688_Est_Angle_complement_t;


/**
 * @brief   Read the current temperature and convert it to degrees Celsius.
 *          The two register bytes are combined according to the sensor-data endian configuration cached in
 * the handle, then converted with the ICM42688 temperature transfer function.
 * @param   pHandle   Pointer to the ICM42688 handle struct.
 * @param   pOutTempC Pointer to the returned temperature in degrees Celsius.
 * @return  true when the sensor data is read successfully, otherwise false.
 */
bool ICM42688_Get_Temperature_C(ICM42688_Handle_t *pHandle, float *pOutTempC);

/**
 * @brief   Read raw accelerometer samples for the X, Y, and Z axes.
 *          All six data bytes are captured in one burst so the three axes belong to the same register
 * snapshot; byte order follows the configured sensor-data endian mode.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   pBuf     Three-element output buffer for raw axis samples.
 * @return  true when the sensor data is read successfully, otherwise false.
 */
bool ICM42688_Get_Accel_XYZ(ICM42688_Handle_t *pHandle, int16_t *pBuf);

/**
 * @brief   Read accelerometer samples and convert them to g.
 *          The function reuses the raw three-axis burst read and applies accel_g_per_lsb from the active
 * full-scale configuration. The scale factor must have been initialized before this call.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   pG       Three-element output buffer for acceleration in g.
 * @return  true when the sensor data is read and scaled successfully, otherwise false.
 */
bool ICM42688_Get_Accel_G(ICM42688_Handle_t *pHandle, float pG[3]);

/**
 * @brief   Read raw gyroscope samples for the X, Y, and Z axes.
 *          All six data bytes are captured in one burst so the three axes belong to the same register
 * snapshot; byte order follows the configured sensor-data endian mode.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   pBuf     Three-element output buffer for raw axis samples.
 * @return  true when the sensor data is read successfully, otherwise false.
 */
bool ICM42688_Get_Gyro_XYZ(ICM42688_Handle_t *pHandle, int16_t *pBuf);

/**
 * @brief   Read gyroscope samples and convert them to degrees per second.
 *          The function reuses the raw three-axis burst read and applies gyro_dps_per_lsb from the active
 * full-scale configuration. The scale factor must have been initialized before this call.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   pDps     Three-element output buffer for angular rate in degrees per second.
 * @return  true when the sensor data is read and scaled successfully, otherwise false.
 */
bool ICM42688_Get_Gyro_DPS(ICM42688_Handle_t *pHandle, float pDps[3]);

/**
 * @brief   Read raw temperature, accelerometer, and gyroscope data in one burst.
 *          One contiguous transaction keeps all seven values temporally aligned and decodes each 16-bit field
 * using the configured sensor-data endian mode.
 * @param   pHandle  Pointer to the ICM42688 handle struct.
 * @param   pOutRaw  Pointer to the returned raw sensor sample.
 * @return  true when the complete burst is read and decoded successfully, otherwise false.
 */
bool ICM42688_Get_Temp_Accel_Gyro_Raw(ICM42688_Handle_t *pHandle, ICM42688_Raw_t *pOutRaw);

/**
 * @brief   Average stationary raw samples to estimate accelerometer and gyroscope offsets.
 *          The device must remain motionless and level throughout collection because movement is accumulated
 * as bias. The Z-axis gravity contribution is removed from the stored accelerometer offset.
 * @param   pHandle               Pointer to the ICM42688 handle struct.
 * @param   pOffsetCalibratedRaw  Pointer to the returned raw offsets.
 * @param   samples               Number of stationary samples to average.
 * @return  true when all requested samples are collected successfully, otherwise false.
 * @note    The accelerometer Z offset excludes one g so stationary gravity remains in later samples.
 */
bool ICM42688_Get_Calibrate_Raw(ICM42688_Handle_t *pHandle, ICM42688_Offset_Raw_t *pOffsetCalibratedRaw,
                                uint32_t samples);

/**
 * @brief   Read a complete sensor sample, apply raw offsets, and convert it to physical units.
 *          Temperature is converted directly, while accelerometer and gyroscope offsets are subtracted in raw
 * units before their configured scale factors are applied.
 * @param   pHandle     Pointer to the ICM42688 handle struct.
 * @param   pOffsetRaw  Pointer to the raw accelerometer and gyroscope offsets.
 * @param   pSampleOut  Pointer to the calibrated and scaled output sample.
 * @return  true when the sample is read and converted successfully, otherwise false.
 */
bool ICM42688_Get_Temp_Accel_Gyro_Scaled(ICM42688_Handle_t *pHandle, const ICM42688_Offset_Raw_t *pOffsetRaw,
                                         ICM42688_Temp_Accel_Gyro_Scaled_t *pSampleOut);

/**
 * @brief   Estimate body roll and pitch with a complementary filter and integrate gyro yaw.
 *          IMU axes are remapped to the requested body orientation before fusion, and invalid orientations
 * are rejected before the attitude output is changed.
 * @param   pHandle          Pointer to the ICM42688 handle struct.
 * @param   orientation      Mounting orientation used to remap IMU axes to body axes.
 * @param   pInputImuScaled  Pointer to the calibrated IMU sample.
 * @param   pAttitudeOut     Pointer to the updated attitude estimate.
 * @param   dtS              Elapsed sample interval in seconds.
 * @return  true when the orientation is valid and the estimate is updated, otherwise false.
 * @note    Yaw has no absolute reference and is integrated from gyroscope data only.
 * @warning dtS is not range-checked; the caller must provide a valid positive sample interval.
 */
bool ICM42688_Get_Est_Angle_Complement(ICM42688_Handle_t *pHandle, ICM42688_Orientation_t orientation,
                                       const ICM42688_Temp_Accel_Gyro_Scaled_t *pInputImuScaled,
                                       ICM42688_Est_Angle_complement_t *pAttitudeOut, float dtS);

#endif /* INC_IMU_SENSORS_ICM42688_DATA_H_ */
