#include <stdio.h>

#include "FreeRTOS.h"
#include "HAL/socuart.h"
#include "pico/stdlib.h"
#include "state.h"
#include "task.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
  debug_printf("Stack overflow in task %s\n", pcTaskName);
  configASSERT(0);
}

void vApplicationMallocFailedHook() {
  debug_printf("Malloc failed\n");
  configASSERT(0);
}

void vTaskBootstrap(void* pvParameters) {
  // start the system by incrementing the system state
  set_state(STATE_CONFIG);
  vTaskSuspend(NULL);
}

void main() {
  socuart_init();
  xTaskCreate(vTaskBootstrap, "Bootstrap Task", 512, NULL, 1, NULL);
  vTaskStartScheduler();
}