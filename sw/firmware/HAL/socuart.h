#pragma once

#include <stdint.h>

// PORTING: Implement these functions in socuart.c

/* @brief initialize the serial port
 */
void socuart_init();

/* @brief send a byte over the serial port
 * @param data the byte to send
 */
void socuart_write_byte(uint8_t data);

/* @brief receive a byte from the serial port
 * @return the byte received
 */
char socuart_get_byte();

/* @brief send a buffer over the serial port
 * @param data the buffer to send
 * @param len the length of the buffer
 */
void socuart_write_buffer(uint8_t* data, int len);