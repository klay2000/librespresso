#pragma once

#include <stdint.h>

/**
 * @brief This task reads values from the ADC channels and stores them
 */
void vADCTask();

/**
 * @brief get most recent adc reading for the specified channel
 * @param channel The channel to read from
 * @return The value of the ADC reading
 */
uint16_t get_adc_reading(uint8_t channel);