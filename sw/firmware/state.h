#pragma once

#define STATES  \
  X(STARTUP, 0) \
  X(CONFIG, 1)  \
  X(RUNNING, 2)

typedef enum {
#define X(n, num) STATE_##n = num,
  STATES
#undef X
} state_t;

/*
 * @brief Get the current state of the system
 * @return the current state
 */
state_t get_state();

/*
 * @brief Set the state of the system
 * @param state the new state
 */
void set_state(state_t state);
