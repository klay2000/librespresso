#pragma once

#include <stdint.h>

#include "config.h"

// TODO: merge with communication.h

typedef enum {
#define X(n, t, num) CFG_VAR_##n = num,
  CFG_VARS
#undef X
} cfg_var_t;
#define UART_PKT_TYPES                     \
  X(HB, handle_hb, 0)                      \
  X(CONFIG, handle_cfg_pkt, 1)             \
  X(DEBUG, empty_handler, 2)               \
  X(REBOOT, reboot_handler, 3)             \
  X(START_SYSTEM, start_system_handler, 4) \
  X(SET_PUMP_PRESSURE, handle_pump_pressure, 5)

// TODO: make it possible to define pinouts at startup via config

typedef enum {
#define X(n, h, num) UART_PKT_##n = num,
  UART_PKT_TYPES
#undef X
} uart_pkt_id_t;

typedef struct {
  uint8_t len;
  uint8_t checksum;
  uart_pkt_id_t id;
  uint8_t data[255];
} uart_pkt_t;

/**
 *@brief Handles a packet received over serial from the SOC
 *@param pkt pointer to the packet to handle
 */
void handle_pkt(uart_pkt_t* pkt);