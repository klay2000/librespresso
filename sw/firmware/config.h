#pragma once

#include <stdint.h>

// add a name and type for each config variable here
// this will be used to generate get/set functions
#define CFG_VARS            \
  X(pump_max_rpm, uint32_t) \
  X(pump_pressure_p, float) \
  X(pump_pressure_i, float) \
  X(pump_pressure_d, float) \
  X(pressure_cal_m, float)  \
  X(pressure_cal_b, float)

// X macro to declare get/set functions for each config variable
#define X(n, t) \
  t get_##n();  \
  void set_##n(t n);
CFG_VARS
#undef X
