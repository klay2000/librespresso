#pragma once

#include "config_types.h"

/**
 *@brief Calculate the output of a linear calibration
 *@param cal the calibration
 *@param x input value
 *@return output value
 */
float linear_cal_calculate(linear_cal_config_t* cal, float x);