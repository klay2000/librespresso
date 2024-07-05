#include "command.h"

#include <stdint.h>

#include "communication.h"
#include "config.h"

// empty packet handler
static void empty_handler(uart_pkt_t* pkt);
// heartbeat packet handler
static void handle_hb(uart_pkt_t* pkt);
// config packet handler
static void handle_cfg_pkt(uart_pkt_t* pkt);

void handle_pkt(uart_pkt_t* pkt) {
  switch (pkt->id) {
#define X(n, h)      \
  case UART_PKT_##n: \
    h(pkt);          \
    break;
    UART_PKT_TYPES
#undef X
  }
}

// packet handlers

static void empty_handler(uart_pkt_t* pkt) {
  // do nothing
}

static void handle_hb(uart_pkt_t* pkt) {
  // send a heartbeat packet back to the SOC
  uart_pkt_t* hb_pkt = get_tx_buffer();
  hb_pkt->len        = 0;
  hb_pkt->id         = 0;
  send_pkt(hb_pkt);
}

static void handle_cfg_pkt(uart_pkt_t* pkt) {
  cfg_var_t var = pkt->data[0];
  uint8_t* data = pkt->data + sizeof(cfg_var_t);

  switch (var) {
#define X(n, t)           \
  case CFG_VAR_##n:       \
    set_##n(*((t*)data)); \
    break;
    CFG_VARS
#undef X
  }
}