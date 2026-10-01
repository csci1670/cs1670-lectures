#include "u_common.h"

int main() {
  for (int i = 0; i < 100; i++) {
    printf("Process hello: before yield\r\n");
    yield();
    printf("Process hello: after yield\r\n");
  }
  printf("Process hello: done after 100 loops\r\n");
  exit();
  while (true) {
  }
}
