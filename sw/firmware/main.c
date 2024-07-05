#include <stdio.h>

#include "FreeRTOS.h"
#include "HAL/ADC.h"
#include "HAL/pump.h"
#include "HAL/socuart.h"
#include "communication.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "task.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  debug_printf("Stack overflow in task %s\n", pcTaskName);
  configASSERT(0);
}

void vApplicationMallocFailedHook() {
  debug_printf("Malloc failed\n");
  configASSERT(0);
}

void vPrintTask(void *pvParameters) {
  while (1) {
    for (int i = 0; i < 8; i++) {
      debug_printf("ADC Channel %d: %d\n", i, get_adc_reading(i));
    }

    int32_t rpm = get_pump_rpm();

    debug_printf("Pump rpm: %d\n", rpm);

    if (rpm > -10 && rpm < 10) {
      set_pump_rpm(1000);
    } else {
      set_pump_rpm(0);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void main() {
  socuart_init();

  // start comm tasks
  xTaskCreate(vSerialRxTask, "Serial RX Task", 512, NULL, 1, NULL);
  xTaskCreate(vSerialTxTask, "Serial TX Task", 512, NULL, 1, NULL);

  // start driver tasks
  xTaskCreate(vPumpTask, "Pump Task", 512, NULL, 1, NULL);
  xTaskCreate(vADCTask, "ADC Task", 512, NULL, 1, NULL);

  // start application tasks
  xTaskCreate(vPrintTask, "Print Task", 512, NULL, 1, NULL);

  debug_printf("tasks started\n");
  vTaskStartScheduler();
}