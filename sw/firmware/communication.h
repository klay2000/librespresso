#pragma once
#include <command.h>

// TODO: merge with command.h

#define TX_BUF_SIZE 5

// Tasks for handling serial communication
void vSerialRxTask(void* pvParameters);
void vSerialTxTask(void* pvParameters);

/**
 * @brief Queues a packet to be sent over serial
 * @param pkt pointer to the packet to send
 */
void send_pkt(uart_pkt_t* pkt);

/*
 * @brief Gets a tx buffer from the pool
 * @return pointer to the packet
 */
uart_pkt_t* get_tx_buffer();

/**
 * @brief Sends a debug message over serial, max length 255
 * @param fmt format string
 * @param ... arguments to format string
 */
void debug_printf(const char* fmt, ...);