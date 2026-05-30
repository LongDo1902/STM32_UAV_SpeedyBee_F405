#ifndef _PID_H_
#define _PID_H_

#include <stdbool.h>
#include <stdint.h>

// PID Gains
typedef struct
{
    float kp;
    float ki;
    float kd;
} pid_gains_t;

// Parameter of PID Controller
typedef struct
{
    pid_gains_t gains;

    float setpoint;
    float input;
    float input_prev;
    float output;

    float error;
    float error_sum;
    float error_deriv;

    uint32_t sample_time;

    float error_sum_max;
    float error_sum_min;
} pid_def_t;

// Cascaded PID controller
typedef struct
{
    pid_def_t outer_loop; // angle
    pid_def_t inner_loop; // rate
} pid_controller_t;

typedef enum
{
    PID_LOOP_OUTER,
    PID_LOOP_INNER
} pid_loop_t;

void pid_init();

uint16_t pid_compute(pid_controller_t *pid, const float setpoint_angle, const float angle,
                     const float rate);

uint8_t pid_get_gains(pid_controller_t *pid, pid_loop_t loop, pid_gains_t *gains);
uint8_t pid_set_gains(pid_controller_t *pid, const pid_loop_t loop, pid_gains_t gains);

void clamp(float *value, const float *max, const float *min);

#endif // _PID_H_
