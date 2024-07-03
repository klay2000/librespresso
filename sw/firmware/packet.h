#pragma once

#include <config.h>
#include <stdint.h>

typedef enum {
#define X(n, t) CFG_VAR_##n,
  CFG_VARS
#undef X
} cfg_var_t;

// add a pkt and it's handler/data here to define a new packet type
#define UART_PKT_TYPES \
  X(PING, handle_ping) \
  X(PONG, handle_pong) \
  X(HB, handle_hb)     \
  X(CONFIG, handle_cfg_pkt, cfg_var_t var, uint32_t val)

// TODO: make it possible to define pinouts at startup via config

typedef enum {
#define X (n, h, ...) UART_PKT_##n,
  UART_PKT_TYPES
#undef X
} uart_pkt_id_t;

/* TODO: finish this, the idea is to make a union with all packet types and then
   pass the union to the handler function, which will cast it to the correct
   type and process it. */