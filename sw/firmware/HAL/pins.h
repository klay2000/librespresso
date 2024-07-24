#pragma once

#include <hardware/spi.h>
#include <hardware/uart.h>

#define MCP3008_SPI_PORT spi0
#define MCP3008_CS_PIN   5
#define MCP3008_SCK_PIN  2
#define MCP3008_MISO_PIN 4
#define MCP3008_MOSI_PIN 3

#define VESC_UART_PORT uart0
#define VESC_TX_PIN    16
#define VESC_RX_PIN    17

#define PUMP_PRESSURE_SENSOR_ADC_CHANNEL 0