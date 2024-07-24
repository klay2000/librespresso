#include "HAL/reboot.h"

#include <stdbool.h>

#include "hardware/watchdog.h"

void reboot_mcu() {
  // reboot the system by setting the watchdog and spinning.
  // this is a bit of a hack, but it works

  watchdog_enable(1, false);
  while (1);
}