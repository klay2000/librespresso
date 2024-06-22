#include "serial.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "constants.h"
#include "pico/stdlib.h"
#include "semphr.h"

SemaphoreHandle_t xSerialOutSemaphore = NULL;
SemaphoreHandle_t xSerialInSemaphore  = NULL;

static SemaphoreHandle_t setup_semaphore();

static SemaphoreHandle_t setup_semaphore() {
  SemaphoreHandle_t semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(semaphore);
  return semaphore;
}

void ts_printf(const char *fmt, ...) {
  if (xSerialOutSemaphore == NULL) {
    xSerialOutSemaphore = setup_semaphore();
  }

  xSemaphoreTake(xSerialOutSemaphore, portMAX_DELAY);

  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);

  xSemaphoreGive(xSerialOutSemaphore);
}

void ts_debug_printf(const char *fmt, ...) {
#ifdef DEBUG
  if (xSerialOutSemaphore == NULL) {
    xSerialOutSemaphore = xSemaphoreCreateBinary();
  }

  xSemaphoreTake(xSerialOutSemaphore, portMAX_DELAY);

  printf("[DEBUG] ");
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);

  xSemaphoreGive(xSerialOutSemaphore);
#endif
}

bool ts_serial_getchar(char *c) {
  if (xSerialInSemaphore == NULL) {
    xSerialInSemaphore = setup_semaphore();
  }

  xSemaphoreTake(xSerialInSemaphore, portMAX_DELAY);

  char result = getchar_timeout_us(0);

  if (result == PICO_ERROR_TIMEOUT) {
    xSemaphoreGive(xSerialInSemaphore);
    return false;
  }

  *c = result;
  xSemaphoreGive(xSerialInSemaphore);
  return true;
}