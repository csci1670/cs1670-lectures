#include "u_common.h"

int main() {
  unsigned int ctr = 0;
  for (int i = 0; i < 4; ++i) {
    printf("Process counter: %d\r\n", ctr++);
  }

  exit();
}
