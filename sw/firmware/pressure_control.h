#pragma once

/*
 * @brief function to set the pressure setpoint
 * @param pressure the pressure setpoint
 */
void set_pressure_setpoint(float pressure);

/*
 * @brief Task to control the pressure from the pump, this task can be started
 * and stopped multiple times
 */
void vTaskPressureControl(void* pvParameters);
