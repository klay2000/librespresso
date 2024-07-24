#include "command.h"

#include <stdint.h>

#include "HAL/reboot.h"
#include "communication.h"
#include "config.h"
#include "state.h"

// empty packet handler
static void empty_handler(uart_pkt_t* pkt);
// heartbeat packet handler
static void handle_hb(uart_pkt_t* pkt);
// config packet handler
static void handle_cfg_pkt(uart_pkt_t* pkt);
// reboot packet handler
static void reboot_handler(uart_pkt_t* pkt);
// start system packet handler
static void start_system_handler(uart_pkt_t* pkt);
// pump pressure packet handler
static void handle_pump_pressure(uart_pkt_t* pkt);

void handle_pkt(uart_pkt_t* pkt) {
  // debug_printf("Received packet, id: %d\n", pkt->id);
  switch (pkt->id) {
#define X(n, h, num) \
  case UART_PKT_##n: \
    h(pkt);          \
    break;
    UART_PKT_TYPES
#undef X
  }
}

static void handle_cfg_pkt(uart_pkt_t* pkt) {
  cfg_var_t var = pkt->data[0];
  uint8_t* data = &(pkt->data[1]);

  switch (var) {
#define X(n, t, num)      \
  case CFG_VAR_##n:       \
    set_##n(*((t*)data)); \
    break;
    CFG_VARS
#undef X
    default:
      debug_printf("Invalid config variable: %d\n", var);
      break;
  }
}

// packet handlers

static void empty_handler(uart_pkt_t* pkt) {
  // do nothing
}

// TODO: properly handle this: probably by feeding the watchdog, will require
// re-implementation of restart_mcu
static void handle_hb(uart_pkt_t* pkt) {
  // send a heartbeat packet back to the SOC
  uart_pkt_t* hb_pkt = get_tx_buffer();
  hb_pkt->len        = 0;
  hb_pkt->id         = UART_PKT_HB;
  send_pkt(hb_pkt);
}

static void reboot_handler(uart_pkt_t* pkt) { reboot_mcu(); }

static void start_system_handler(uart_pkt_t* pkt) {
  // start the system by incrementing the system state
  set_state(STATE_RUNNING);
}

static void handle_pump_pressure(uart_pkt_t* pkt) {  // TODO: implement
}