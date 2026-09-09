#include "u_common.h"

#include "u_klib.h"
#include "u_types.h"

void printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  // calls the function which address is contained in F_VPRINTF
  (*(void (**)(const char*, va_list))F_VPRINTF)(fmt, args);

  va_end(args);
};

void exit() {
  return;
}
