#include "state.h"

#include "FreeRTOS.h"
#include "HAL/ADC.h"
#include "HAL/pump.h"
#include "communication.h"
#include "pressure_control.h"
#include "task.h"

// macro to make state transitions more readable
#define X_TO_Y(x, y) state == x&& new_state == y

static state_t state = STATE_STARTUP;

static void state_transition(state_t new_state);
static void debug_task(void* pvParameters);

state_t get_state() { return state; }
void set_state(state_t new_state) {
  state_transition(new_state);
  state = new_state;
}

static void debug_task(void* pvParameters) {
  while (1) {
    debug_printf("State: %d\n", state);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// TODO: if this function gets too long, consider breaking it or using some
// macros to make it more readable
static void state_transition(state_t new_state) {
  if (X_TO_Y(STATE_STARTUP, STATE_CONFIG)) {
    // start comm tasks
    xTaskCreate(vSerialRxTask, "Serial RX Task", 512, NULL, 1, NULL);
    xTaskCreate(vSerialTxTask, "Serial TX Task", 512, NULL, 1, NULL);
  } else if (X_TO_Y(STATE_CONFIG, STATE_RUNNING)) {
    // start driver tasks
    xTaskCreate(vPumpTask, "Pump Task", 512, NULL, 1, NULL);
    xTaskCreate(vADCTask, "ADC Task", 512, NULL, 1, NULL);

    // start application tasks
    // xTaskCreate(debug_task, "Debug Task", 512, NULL, 1, NULL);
    xTaskCreate(vTaskPressureControl, "Pressure Control Task", 512, NULL, 1,
                NULL);
  }

  if (new_state != state) {
    state = new_state;
  }
}

#undef X_TO_Y