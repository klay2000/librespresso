#include "linear_cal.h"

#include "config_types.h"

float linear_cal_calculate(linear_cal_config_t* cal, float x) {
  return cal->slope * x + cal->offset;
}