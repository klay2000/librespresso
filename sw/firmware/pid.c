#include "pid.h"

#include <math.h>

#include "FreeRTOS.h"
#include "config_types.h"

void pid_init(pid_controller_t* pid, pid_config_t conf) {
  pid->conf      = conf;
  pid->setpoint  = 0;
  pid->out       = 0;
  pid->integral  = 0;
  pid->error     = 0;
  pid->last_tick = 0;
}

void pid_set_setpoint(pid_controller_t* pid, float setpoint) {
  pid->setpoint = setpoint;
}

float pid_update(pid_controller_t* pid, float input) {
  TickType_t now = xTaskGetTickCount();
  TickType_t dt  = now - pid->last_tick;

  float new_error = pid->setpoint - input;
  float p_term    = pid->conf.kp * new_error;
  pid->integral += (new_error + pid->error) / 2 * dt;  // trapezoidal rule
  float i_term = pid->conf.ki * pid->integral;
  float d_term = pid->conf.kd * (pid->error - new_error) / dt;

  pid->out = p_term + i_term + d_term;
  pid->out = fminf(fmaxf(pid->out, pid->conf.out_min), pid->conf.out_max);

  pid->error     = new_error;
  pid->last_tick = now;

  return pid->out;
}

float get_pid_output(pid_controller_t* pid) { return pid->out; }