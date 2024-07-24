#pragma once

#include <stdint.h>

#include "config_types.h"

// add a name and type for each config variable here
// this will be used to generate get/set functions
#define CFG_VARS                        \
  X(pump_max_rpm, uint32_t, 0)          \
  X(pump_pressure_pid, pid_config_t, 1) \
  X(pump_pressure_cal, linear_cal_config_t, 2)

// X macro to declare get/set functions for each config variable
#define X(n, t, num) \
  t get_##n();       \
  void set_##n(t n);
CFG_VARS
#undef X
