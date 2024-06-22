#include <stdio.h>

#include "FreeRTOS.h"
#include "HAL/ADC.h"
#include "HAL/pump.h"
#include "HAL/serial.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "task.h"
#include "tusb.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  ts_debug_printf("Stack overflow in task %s\n", pcTaskName);
  configASSERT(0);
}

void vApplicationMallocFailedHook() {
  ts_debug_printf("Malloc failed\n");
  configASSERT(0);
}

void vPrintTask(void *pvParameters) {
  while (1) {
    for (int i = 0; i < 8; i++) {
      ts_debug_printf("ADC Channel %d: %d\n", i, get_adc_reading(i));
    }

    int32_t rpm = get_pump_rpm();

    ts_debug_printf("Pump rpm: %d\n", rpm);

    if (rpm > -10 && rpm < 10) {
      set_pump_rpm(1000);
    } else {
      set_pump_rpm(0);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void main() {
  stdio_init_all();
  while (!tud_cdc_available());
  ts_debug_printf("ready...\n");

  // start driver tasks
  xTaskCreate(vPumpTask, "Pump Task", 512, NULL, 1, NULL);
  xTaskCreate(vADCTask, "ADC Task", 512, NULL, 1, NULL);

  // start application tasks
  xTaskCreate(vPrintTask, "Print Task", 512, NULL, 1, NULL);

  ts_debug_printf("tasks started\n");
  vTaskStartScheduler();
}