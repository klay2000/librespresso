#pragma once

#include <stdint.h>

typedef struct __attribute__((packed)) {
  float kp;
  float ki;
  float kd;
  float out_min;
  float out_max;
} pid_config_t;

typedef struct __attribute__((packed)) {
  float slope;
  float offset;
} linear_cal_config_t;
