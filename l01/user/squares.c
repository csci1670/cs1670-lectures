#include "u_common.h"

/* squares.c - Compute squares of integers from 1 to n and their differences
 *
 * Following in the tradition of a program run on many early computers to test
 * that their arithmetic units were working correctly, this program computes the
 * squares of integers from 1 to n and prints each integer, its square, and the
 * difference between the current square and the previous square. The output is
 * formatted in a tabular form.
 */

int main(void) {
  int n = 100;  // Number of squares to compute
  int p = 0;    // Previous square value

  for (int i = 1; i <= n; i++) {
    int s = i * i;  // Compute the square of i
    // Print i, square, and difference from previous square
    printf("%d\t%d\t%d\n", i, s, s - p);
    p = s;  // Update previous square value
  }

  exit();
}
