#include "pid.h"

uint16_t
pid_compute(pid_controller_t *pid, const float setpoint_angle, const float angle, const float rate)
{
    /* OUTER CONTROLLER FOR ANGLE */
    pid->outer_loop.setpoint = setpoint_angle;
    pid->outer_loop.input    = angle;

    pid->outer_loop.error = pid->outer_loop.setpoint - pid->outer_loop.input;
    pid->outer_loop.error_sum += pid->outer_loop.error * pid->outer_loop.sample_time;
    pid->outer_loop.error_deriv = -rate;

    // anti wind-up for error outer controller
    clamp(&pid->outer_loop.error_sum, &pid->outer_loop.error_sum_max,
          &pid->outer_loop.error_sum_min);

    // TODO: filer D gains

    pid->outer_loop.output = pid->outer_loop.gains.kp * pid->outer_loop.error +
                             pid->outer_loop.gains.ki * pid->outer_loop.error_sum +
                             pid->outer_loop.gains.kd * pid->outer_loop.error_deriv;

    /* INNER CONTROLLER FOR RATE */
    pid->inner_loop.setpoint = pid->outer_loop.output;
    pid->inner_loop.input    = rate;

    pid->inner_loop.error = pid->inner_loop.setpoint - pid->inner_loop.input;
    pid->inner_loop.error_sum += pid->inner_loop.error * pid->inner_loop.sample_time;

    pid->inner_loop.error_deriv = -(pid->inner_loop.input - pid->inner_loop.input_prev);
    pid->inner_loop.error_deriv = pid->inner_loop.error_deriv / pid->inner_loop.sample_time;
    pid->inner_loop.input_prev  = pid->inner_loop.input;

    // anti wind-up for error inner controller
    clamp(&pid->inner_loop.error_sum, &pid->inner_loop.error_sum_max,
          &pid->inner_loop.error_sum_min);

    // TODO: filer D gains

    pid->inner_loop.output = pid->inner_loop.gains.kp * pid->inner_loop.error +
                             pid->inner_loop.gains.ki * pid->inner_loop.error_sum +
                             pid->inner_loop.gains.kd * pid->inner_loop.error_deriv;

    return pid->inner_loop.output;
}

uint8_t
pid_get_gains(pid_controller_t *pid, pid_loop_t loop, pid_gains_t *gains)
{
    if (!pid || !gains) {
        return false;
    }

    switch (loop) {
        case PID_LOOP_OUTER:
            *gains = pid->outer_loop.gains;
            break;
        case PID_LOOP_INNER:
            *gains = pid->inner_loop.gains;
            break;
        default:
            return false;
    }

    return true;
}

uint8_t
pid_set_gains(pid_controller_t *pid, const pid_loop_t loop, pid_gains_t gains)
{
    if (!pid) {
        return false;
    }

    if (gains.kp < 0 || gains.ki < 0 || gains.kd < 0) {
        return false;
    }

    switch (loop) {
        case PID_LOOP_OUTER:
            pid->outer_loop.gains = gains;
            break;
        case PID_LOOP_INNER:
            pid->inner_loop.gains = gains;
            break;
        default:
            return false;
    }

    return true;
}


void
clamp(float *value, const float *max, const float *min)
{
    if (*value > *max) {
        *value = *max;
    }
    else if (*value < *min) {
        *value = *min;
    }
}