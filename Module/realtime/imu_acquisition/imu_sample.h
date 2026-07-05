#ifndef IMU_SAMPLE_H
#define IMU_SAMPLE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint32_t timestamp;

    float gyro_dps[3];
    float accel_g[3];
    float temp_c;

    bool healthy;
    bool new_sample;
} IMU_Sample_t;

#endif