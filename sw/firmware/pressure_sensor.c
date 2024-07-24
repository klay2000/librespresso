#include "pressure_sensor.h"

#include "HAL/ADC.h"
#include "config_types.h"
#include "linear_cal.h"

void pressure_sensor_init(pressure_sensor_t* sensor, uint8_t adc_channel,
                          linear_cal_config_t* cal) {
  sensor->adc_channel = adc_channel;
  sensor->cal         = cal;
}

float pressure_sensor_read(pressure_sensor_t* sensor) {
  float adc_value = get_adc_reading(sensor->adc_channel);
  return linear_cal_calculate(sensor->cal, adc_value);
}