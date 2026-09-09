/*
 * pi.c -- Estimate pi with the spigot algorithm, storing the digits as
 *         binary-coded decimal (BCD) bytes.
 *
 * The point of the BCD form is that the computed value lives in memory as
 * bytes.  Each byte holds a single decimal digit (value 0..9), a format called
 * "unpacked BCD".  The global array `pi_bcd` contains these digits as bytes
 * and you can inspect its contents (e.g., in GDB): the first byte is the
 * integer part (3), and the rest are the digits after the decimal point.
 *
 * Converting those digits to human-readable ASCII happens in `print_pi()`,
 * which is only compiled when PI_PRINT is set to 1, since you won't have a
 * `printf` function initially.
 *
 * In order to compute pi digit by digit and without using floating point, we
 * use the spigot algorithm of Rabinowitz & Wagon (1995).
 *
 * ----------------------------------------------------------------------------
 * THE ALGORITHM: a "spigot" for the digits of pi (Rabinowitz & Wagon, 1995)
 * ----------------------------------------------------------------------------
 *
 * The code never represent pi as a single floating-point number.  Instead, the
 * code produces its decimal digits one at a time using only integer
 * add/multiply/divide/mod. Such a method is called a "spigot" algorithm
 * because digits trickle out the end like water from a tap.
 *
 * It is built on this rapidly-converging series for pi:
 *
 *       pi = 2 + (1/3)(2 + (2/5)(2 + (3/7)(2 + (4/9)(2 + ...))))
 *
 * Read the nesting from the inside out.  Equivalently, pi is the sum
 *
 *       pi = sum_{i=0..inf}  2 * w_i,   where  w_i = prod_{k=1..i} k/(2k+1).
 *
 * Think of this as a number written in a *mixed-radix* (non-uniform base)
 * positional system.  The code keeps an array `spigot_state[]` where every
 * entry starts at the digit 2, and entry i sits in a "place" whose value is
 * w_i.  The place values shrink quickly, which is why only ~ (10/3) * D
 * entries are needed to pin down D decimal digits (each base-10 digit costs
 * about log2(10)/... worth of these places; the constant works out to 10/3).
 *
 * To read off decimal digits we do ordinary "long multiplication by 10" of the
 * whole mixed-radix number, repeatedly:
 *
 *   1. Multiply every entry by 10.
 *   2. Normalize from the least significant place (the far right) toward the
 *      most significant.  At place j the legal range of a digit is
 *      [0, 2j+1), so we carry the overflow left:
 *
 *          q          = state[j] / (2j+1)   // how many times we overflow
 *          state[j]   = state[j] % (2j+1)   // remainder stays in place j
 *          state[j-1] += q * j              // carry, scaled by the radix link
 *
 *      The carry is multiplied by j because the link between place j and place
 *      j-1 is the factor k/(2k+1) with k = j: a bundle of (2j+1) units at
 *      place j is worth exactly j units at place j-1.
 *   3. Place 0 is an ordinary base-10 place, so its overflow IS the next
 *      decimal digit:
 *
 *          digit    = state[0] / 10
 *          state[0] = state[0] % 10
 *
 * One wrinkle: the "digit" produced in step 3 can momentarily come out as 9 or
 * even 10, because a later step may generate a carry that ripples back into
 * digits we have already emitted (think 0.1999... rounding up to 0.2).  We
 * handle this exactly (no rounding error) with the classic pre-digit buffer:
 *
 *   - A freshly produced 9 is *held* (counted in `nines`) rather than emitted,
 *     because a future carry could turn it into a 0.
 *   - A 10 means such a carry happened: the previously held digit is bumped up
 *     by one and every held 9 collapses to 0.
 *   - Any digit 0..8 is "safe": it lets us finally flush the previous held
 *     digit and all the pending 9s.
 *
 * Because we always emit the *previous* settled digit, the very first value
 * pushed out is a placeholder leading 0; we skip it when formatting.  The next
 * digit is the integer part (3), and the rest are the fractional digits.
 *
 * Cost: O(D^2) integer operations and O(D) memory for D digits -- fine for the
 * modest digit counts used.
 */

#include "drivers/uart.h"
#include "printf.h"

/* ---- Configuration --------------------------------------------------------
 */

/* Number of digits AFTER the decimal point. */
#define PI_DIGITS 1000

/* Set to 1 to print the result, 0 to compute silently (value stays in memory).
 */
#define PI_PRINT 0
/* How many digits to print per line. */
#define LINE_LEN 134

/* ---- Derived sizes --------------------------------------------------------
 */

/* Total significant digits: the leading "3" plus the fractional digits. */
#define TOTAL_DIGITS (PI_DIGITS + 1)

/* ~10/3 spigot array entries are needed per decimal digit; +1 for margin. */
#define ARRAY_LEN (10 * TOTAL_DIGITS / 3 + 1)

/* Bytes of BCD output: one for the integer part, PI_DIGITS for the fraction. */
#define BCD_LEN (PI_DIGITS + 1)

typedef long pi_int;

/* ---- Globals (inspectable in memory even with output disabled) ------------
 */

/* The spigot working number. */
pi_int spigot_state[ARRAY_LEN];

/*
 * The result as unpacked BCD: one decimal digit (0..9) per byte.
 *   pi_bcd[0]            = integer part (3)
 *   pi_bcd[1 .. PI_DIGITS] = digits after the decimal point
 */
unsigned char pi_bcd[BCD_LEN];

/* Running count of digits emitted by the spigot, including the leading
 * placeholder zero (see put_digit). File-scope so the emit helper can share it
 * with compute_pi. */
static int emitted;

/*
 * Store one produced digit into the BCD array.
 *
 * The spigot's pre-digit pipeline emits a spurious leading 0 first, then the
 * integer part, then the fractional digits.  We drop that placeholder (emitted
 * index 0) and store everything after it, bounded by the array size.
 */
static void put_digit(int digit) {
  int idx = emitted - 1; /* -1 skips the leading placeholder zero */
  if (idx >= 0 && idx < BCD_LEN) {
    pi_bcd[idx] = (unsigned char)digit;
  }
  emitted++;
}

/* ---- The calculation ---------------------------------
 */

void compute_pi(void) {
  for (int i = 0; i < ARRAY_LEN; i++) {
    spigot_state[i] = 2;
  }

  int predigit = 0; /* last settled digit, not yet emitted */
  int nines = 0;    /* count of held 9s awaiting a carry verdict */
  emitted = 0;

  /* TOTAL_DIGITS + 1 steps: the +1 keeps the pipeline one digit ahead. */
  for (int step = 0; step < TOTAL_DIGITS + 1; step++) {
    /* Multiply the whole mixed-radix number by 10. */
    for (int i = 0; i < ARRAY_LEN; i++) {
      spigot_state[i] *= 10;
    }

    /* Normalize/carry from the least significant place leftward. */
    for (int j = ARRAY_LEN - 1; j >= 1; j--) {
      pi_int q = spigot_state[j] / (2 * j + 1);
      spigot_state[j] = spigot_state[j] % (2 * j + 1);
      spigot_state[j - 1] += q * j;
    }

    /* Place 0 is base 10: its overflow is the tentative next digit. */
    pi_int digit = spigot_state[0] / 10;
    spigot_state[0] = spigot_state[0] % 10;

    if (digit == 9) {
      nines++; /* hold: a later carry might turn these into 0s */
    } else if (digit == 10) {
      /* Carry arrived: bump the held digit, held 9s become 0s. */
      put_digit(predigit + 1);
      for (int k = 0; k < nines; k++) {
        put_digit(0);
      }
      predigit = 0;
      nines = 0;
    } else {
      /* Safe digit (0..8): flush the previous digit and pending 9s. */
      put_digit(predigit);
      for (int k = 0; k < nines; k++) {
        put_digit(9);
      }
      nines = 0;
      predigit = (int)digit;
    }
  }
  /* Flush the tail of the pipeline. */
  put_digit(predigit);
  for (int k = 0; k < nines; k++) {
    put_digit(9);
  }
}

/* ---- Presentation (BCD -> ASCII), enable via PI_PRINT --------------
 */

#if PI_PRINT
static void print_pi(void) {
  /* Convert each stored digit d to the ASCII character '0' + d. */
  printf("pi ~= %c.", '0' + pi_bcd[0]); /* integer part, then the point */
  for (int i = 1; i < BCD_LEN; i++) {
    printf("%c", '0' + pi_bcd[i]);
    /* Offset by six because of the leading "pi ~= 3." on the first line*/
    if ((i + 8) % LINE_LEN == 0) {
      printf("\r\n");
    }
  }
  printf("\r\n");
}
#endif

/* ---- main(), actual program entry point --------------
 */

int pi_main(void) {
  compute_pi();

#if PI_PRINT
  uart_init();
  print_pi();
#endif

  return 0;
}
