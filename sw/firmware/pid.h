#pragma once
#include "FreeRTOS.h"
#include "config_types.h"

typedef struct {
  pid_config_t conf;
  float setpoint;
  float out;
  float integral;
  float error;
  TickType_t last_tick;
} pid_controller_t;

/*
 * @brief Initialize the PID controller
 * @param pid: pointer to the PID controller
 * @param conf: the configuration for the PID controller
 */
void pid_init(pid_controller_t* pid, pid_config_t conf);

/*
 * @brief Set the setpoint of the PID controller
 * @param pid: pointer to the PID controller
 * @param setpoint: the new setpoint
 */
void pid_set_setpoint(pid_controller_t* pid, float setpoint);

/*
 * @brief Update the PID controller
 * @param pid: pointer to the PID controller
 * @param input: the new input value
 */
float pid_update(pid_controller_t* pid, float input);

/*
 * @brief Get the output of the PID controller
 * @param pid: pointer to the PID controller
 */
float get_pid_output(pid_controller_t* pid);