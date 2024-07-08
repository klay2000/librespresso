#include "socuart.h"

#include <stdio.h>

#include "tusb.h"

void socuart_init() {
  stdio_init_all();
  stdio_set_translate_crlf(&stdio_usb, false);
}

void socuart_write_byte(uint8_t data) { putchar(data); }

char socuart_get_byte() { return getchar(); }

void socuart_write_buffer(uint8_t* data, int len) {
  for (int i = 0; i < len; i++) {
    putchar(data[i]);
  }
}