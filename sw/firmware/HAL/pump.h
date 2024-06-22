#pragma once

#include <stdint.h>

/**
 * @brief This task sets up the pump and handles interfacing with it
 */
void vPumpTask();

/**
 * @brief get the most recently read pump rpm
 * @return The most recently read pump rpm
 */
uint32_t get_pump_rpm();

/**
 * @brief set the pump speed
 * @param speed The speed to set the pump to
 */
void set_pump_rpm(int32_t rpm);