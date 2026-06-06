#include "filter/icm42688_lpf.h"

#define ICM42688_LPF_MIN_DT_S 0.000001f
#define ICM42688_LPF_MAX_DT_S 0.1f
#define ICM42688_LPF_PI 3.14159265358979323846f



static bool
ICM42688_PT1_Init(ICM42688_PT1_Filter_t *filter, float cutoff_freq)
{
    if (!filter || cutoff_freq <= 0.0f) {
        return false;
    }

    filter->prev_filtered_val = 0.0f;
    filter->cutoff_hz         = cutoff_freq;
    filter->is_initialized    = false;

    return true;
}



static bool
ICM42688_PT1_Reset(ICM42688_PT1_Filter_t *filter)
{
    if (!filter)
        return false;

    filter->prev_filtered_val = 0.0f;
    filter->is_initialized    = false;

    return true;
}



static float
ICM42688_PT1_Update(ICM42688_PT1_Filter_t *filter, float input, float dt_s)
{
    if (!filter)
        return input;

    if (dt_s < ICM42688_LPF_MIN_DT_S) {
        dt_s = ICM42688_LPF_MIN_DT_S;
    }
    else if (dt_s > ICM42688_LPF_MAX_DT_S) {
        dt_s = ICM42688_LPF_MAX_DT_S;
    }

    if (!filter->is_initialized) {
        filter->prev_filtered_val = input;
        filter->is_initialized    = true;
        return input;
    }

    // Low pass filter formula: y[i] = y[i-1] + alpha * (x[i] - y[i-1])
    float rc    = 1.0f / (2.0f * ICM42688_LPF_PI * filter->cutoff_hz);
    float alpha = dt_s / (dt_s + rc);
    filter->prev_filtered_val += alpha * (input - filter->prev_filtered_val);

    return filter->prev_filtered_val; // Return the filtered value
}



bool
ICM42688_IMU_LPF_Init(ICM42688_IMU_LPF_t *filter, float nominal_dt_s, float gyro_cutoff_hz,
                      float accel_cutoff_hz)
{
    if (!filter)
        return false;

    if (nominal_dt_s <= 0.0f || gyro_cutoff_hz <= 0.0f || accel_cutoff_hz <= 0.0f)
        return false;

    // Avoid silly cutoff values above Nyquist freq
    float sample_rate_hz = 1.0f / nominal_dt_s;
    float nyquist_hz     = sample_rate_hz / 2.0f;

    if ((gyro_cutoff_hz > nyquist_hz) || (accel_cutoff_hz > nyquist_hz))
        return false;

    filter->nominal_dt_s    = nominal_dt_s;
    filter->gyro_cutoff_hz  = gyro_cutoff_hz;
    filter->accel_cutoff_hz = accel_cutoff_hz;
    filter->is_initialized  = false;

    for (uint8_t i = 0; i < 3U; i++) {
        if (!ICM42688_PT1_Init(&filter->gyro_lpf[i], gyro_cutoff_hz))
            return false;

        if (!ICM42688_PT1_Init(&filter->accel_lpf[i], accel_cutoff_hz))
            return false;
    }

    filter->is_initialized = true;

    return true;
}



bool
ICM42688_IMU_LPF_Reset(ICM42688_IMU_LPF_t *filter)
{
    if (!filter)
        return false;
    for (uint8_t i = 0; i < 3U; i++) {
        if (!ICM42688_PT1_Reset(&filter->gyro_lpf[i]))
            return false;

        if (!ICM42688_PT1_Reset(&filter->accel_lpf[i]))
            return false;
    }

    return true;
}



bool
ICM42688_IMU_LPF_Update(ICM42688_IMU_LPF_t *filter, const ICM42688_Temp_Accel_Gyro_Scaled_t *input,
                        ICM42688_Temp_Accel_Gyro_Scaled_t *output)
{
    if (!filter || !input || !output)
        return false;

    if (!filter->is_initialized)
        return false;

    return ICM42688_IMU_LPF_Update_Dt(filter, input, output, filter->nominal_dt_s);
}



bool
ICM42688_IMU_LPF_Update_Dt(ICM42688_IMU_LPF_t                      *filter,
                           const ICM42688_Temp_Accel_Gyro_Scaled_t *input,
                           ICM42688_Temp_Accel_Gyro_Scaled_t *output, float dt_s)
{
    if (!filter || !input || !output)
        return false;

    if (!filter->is_initialized)
        return false;

    output->temp_c = input->temp_c;

    for (uint8_t i = 0; i < 3U; i++) {
        output->gyro_dps[i] = ICM42688_PT1_Update(&filter->gyro_lpf[i], input->gyro_dps[i], dt_s);

        output->accel_g[i] = ICM42688_PT1_Update(&filter->accel_lpf[i], input->accel_g[i], dt_s);
    }

    return true;
}