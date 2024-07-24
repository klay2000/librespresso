#pragma once

#include "HAL/pins.h"
#include "config_types.h"

typedef struct pressure_sensor_t {
  uint8_t adc_channel;
  linear_cal_config_t* cal;
} pressure_sensor_t;

/**
 * @brief Initialize the pressure sensor
 * @param sensor the sensor to initialize
 * @param adc_channel the ADC channel the sensor is connected to
 * @param cal the calibration for the sensor from ADC value to pressure in bar
 */
void pressure_sensor_init(pressure_sensor_t* sensor, uint8_t adc_channel,
                          linear_cal_config_t* cal);

/**
 * @brief Read the pressure from the sensor
 * @param sensor the sensor to read
 * @return the pressure in bar
 */
float pressure_sensor_read(pressure_sensor_t* sensor);