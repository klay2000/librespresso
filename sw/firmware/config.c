#include "config.h"

#include <stdint.h>

#include "command.h"

// X macro to declare each config variable,
#define X(n, t) static t _##n = 0;
CFG_VARS
#undef X

// X macro to define get/set functions for each config variable
#define X(n, t)                \
  t get_##n() { return _##n; } \
  void set_##n(t n) { _##n = n; }
CFG_VARS
#undef X