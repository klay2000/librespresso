#include "pressure_control.h"

#include "FreeRTOS.h"
#include "HAL/pins.h"
#include "HAL/pump.h"
#include "communication.h"
#include "config.h"
#include "linear_cal.h"
#include "pid.h"
#include "pressure_sensor.h"

#define PUMP_PRESSURE_UPDATE_RATE 10

static pid_controller_t pressure_pid     = {};
static pressure_sensor_t pressure_sensor = {};

void set_pressure_setpoint(float pressure) {
  pid_set_setpoint(&pressure_pid, pressure);
}

void vTaskPressureControl(void* pvParameters) {
  linear_cal_config_t cal = get_pump_pressure_cal();
  pid_config_t pid_config = get_pump_pressure_pid();

  pressure_sensor_init(&pressure_sensor, PUMP_PRESSURE_SENSOR_ADC_CHANNEL,
                       &cal);

  pid_init(&pressure_pid, pid_config);

  while (1) {
    float pressure = pressure_sensor_read(&pressure_sensor);
    float output   = pid_update(&pressure_pid, pressure);

    debug_printf("Pressure: %f, Output: %f\n", pressure, output);

    set_pump_rpm(output);

    vTaskDelay(pdMS_TO_TICKS((int)(1.0f / PUMP_PRESSURE_UPDATE_RATE)));
  }
}