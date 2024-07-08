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
  // debug_printf("Stack overflow in task %s\n", pcTaskName);
  configASSERT(0);
}

void vApplicationMallocFailedHook() {
  debug_printf("Malloc failed\n");
  configASSERT(0);
}

void vPrintTask(void *pvParameters) {
  int32_t set_rpm = 0;
  while (1) {
    // for (uint8_t i = 0; i < 8; i++) {
    //   debug_printf("ADC Channel %u: %u\n", i, get_adc_reading(i));
    // }

    debug_printf("adc 0: %u\n", get_adc_reading(0));

    int32_t rpm = get_pump_rpm();

    debug_printf("Pump rpm: %u\n", rpm);

    if (set_rpm > 10) {
      set_pump_rpm(1000 - ((set_rpm - 10) * 100));
    } else {
      set_pump_rpm(set_rpm * 100);
    }
    set_rpm++;
    set_rpm %= 20;

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

  // // start application tasks
  xTaskCreate(vPrintTask, "Print Task", 512, NULL, 1, NULL);

  // printf("tasks started\n");

  // debug_printf("tasks started\n");

  vTaskStartScheduler();
}