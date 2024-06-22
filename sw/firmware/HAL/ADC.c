#include "HAL/ADC.h"

#include "FreeRTOS.h"
#include "HAL/pins.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "task.h"

uint16_t adc_readings[8];  // 8 channels

static void adc_init();
static uint16_t adc_read(uint8_t channel);

static void adc_init() {
  // Initialize SPI
  spi_init(MCP3008_SPI_PORT, 2000000);
  spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  gpio_set_function(MCP3008_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(MCP3008_MISO_PIN, GPIO_FUNC_SPI);
  gpio_set_function(MCP3008_MOSI_PIN, GPIO_FUNC_SPI);

  // Initialize CS pin
  gpio_init(MCP3008_CS_PIN);
  gpio_set_dir(MCP3008_CS_PIN, GPIO_OUT);
  gpio_put(MCP3008_CS_PIN, 1);
}

static uint16_t adc_read(uint8_t channel) {
  uint8_t tx_data[3] = {0x01, 0x80 | (channel << 4), 0x00};
  uint8_t rx_data[3] = {0, 0, 0};
  gpio_put(MCP3008_CS_PIN, 0);
  spi_write_read_blocking(MCP3008_SPI_PORT, tx_data, rx_data, 3);
  gpio_put(MCP3008_CS_PIN, 1);

  return ((rx_data[1] & 0x03) << 8) | rx_data[2];
}

// PORTING: This task reads from the adc channels and dumps them into the
// adc_readings array
void vADCTask() {
  adc_init();
  uint16_t t = 0;
  while (1) {
    for (int i = 0; i < 8; i++) {
      adc_readings[i] = adc_read(i);
      // printf("ADC Channel %d: %d\n", i, adc_readings[i]);
    }
    // printf("loop: %u, HW: %u\n", t++, uxTaskGetStackHighWaterMark(NULL));
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

uint16_t get_adc_reading(uint8_t channel) { return adc_readings[channel]; }