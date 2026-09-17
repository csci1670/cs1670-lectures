#include "u_common.h"
#include "u_types.h"

int main() {
  unsigned int limit = 2 << 27;  // ~134M
  unsigned int prime_count = 0;

  for (unsigned int i = 2; i < limit; i++) {
    bool prime = true;
    for (unsigned int j = 2; j < i; j++) {
      if (i % j == 0) {
        // not a prime, try next i
        prime = false;
        break;
      }
    }
    if (prime) {
      prime_count++;
      if (prime_count % 1000 == 0) {
        printf(
            "primecheck: Found another 1000 primes; last one was "
            "%d!\r\n",
            i);
      }
    }
  }
  exit();
}
