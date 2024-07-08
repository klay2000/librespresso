#include "communication.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "HAL/socuart.h"
#include "command.h"
#include "queue.h"

void vSerialRxTask(void* pvParameters) {
  static uart_pkt_t rx_pkt;
  static uint8_t checksum = 0;

  while (1) {
    // wait for 0x02 since packet hasn't started
    uint8_t data = 0;
    while (data != 0x02) {
      data = socuart_get_byte();
    }
    // start of packet
    rx_pkt.len      = socuart_get_byte();
    rx_pkt.checksum = socuart_get_byte();
    rx_pkt.id       = socuart_get_byte();

    checksum = rx_pkt.checksum - rx_pkt.id;

    for (int i = 0; i < rx_pkt.len; i++) {
      rx_pkt.data[i] = socuart_get_byte();
      checksum -= rx_pkt.data[i];
    }
    if (socuart_get_byte() == 0x03 && rx_pkt.checksum == 0) {
      handle_pkt(&rx_pkt);
    } else {
      // TODO: handle error in a way that isn't just ignoring the packet
    }
  }
}

// vars for TX side
static uart_pkt_t tx_buf[TX_BUF_SIZE];
static QueueHandle_t free_pkt_queue;
static QueueHandle_t tx_queue;

void vSerialTxTask(void* pvParameters) {
  free_pkt_queue = xQueueCreate(TX_BUF_SIZE, sizeof(uart_pkt_t*));
  tx_queue       = xQueueCreate(TX_BUF_SIZE, sizeof(uart_pkt_t*));
  for (int i = 0; i < TX_BUF_SIZE; i++) {
    uart_pkt_t* pkt = &tx_buf[i];
    xQueueSend(free_pkt_queue, &pkt, portMAX_DELAY);
  }

  while (1) {
    uart_pkt_t* pkt;
    // printf("waiting for packet to send\n");
    xQueueReceive(tx_queue, &pkt, portMAX_DELAY);
    // printf("got packet to send\n");

    // calculate checksum
    pkt->checksum = pkt->id;
    for (int i = 0; i < pkt->len; i++) {
      pkt->checksum += pkt->data[i];
    }
    // printf("writing packet\n");
    socuart_write_byte(0x02);
    socuart_write_buffer((uint8_t*)pkt, pkt->len + 3);
    socuart_write_byte(0x03);
    // printf("done\n");

    xQueueSend(free_pkt_queue, &pkt, 0);  // return pkt to pool
  }
}

void send_pkt(uart_pkt_t* pkt) { xQueueSend(tx_queue, &pkt, portMAX_DELAY); }

uart_pkt_t* get_tx_buffer() {
  uart_pkt_t* pkt;
  xQueueReceive(free_pkt_queue, &pkt, portMAX_DELAY);
  return pkt;
}

void debug_printf(const char* fmt, ...) {
  uart_pkt_t* pkt = get_tx_buffer();

  // todo: modify to split into multiple packets if needed

  va_list args;
  va_start(args, fmt);
  vsnprintf(pkt->data, sizeof(pkt->data), fmt, args);
  va_end(args);
  pkt->len = strlen(pkt->data);
  pkt->id  = UART_PKT_DEBUG;

  send_pkt(pkt);
}
