#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief thread safe printf function for serial output
 * @param fmt The format string
 */
void ts_printf(const char *fmt, ...);

/**
 * @brief thread safe printf function for debug output, prints if DEBUG is
 * defined
 * @param fmt The format string
 */
void ts_debug_printf(const char *fmt, ...);

/**
 * @brief thread safe non-blocking getchar function for serial input
 * @param c The character to read into
 * @return true on success, false when no character is available
 */
bool ts_serial_getchar(char *c);