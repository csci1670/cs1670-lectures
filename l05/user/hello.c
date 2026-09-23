#include "u_common.h"

int main() {
  for (int i = 0; i < 50; i++) {
    printf("Process hello: before yield\r\n");
    yield();
    printf("Process hello: after yield\r\n");
  }
  exit();
  while (true) {
  }
}
