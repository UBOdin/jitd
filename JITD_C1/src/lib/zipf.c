#include <math.h>
#include <assert.h>

#include "util.h"
#include "zipf.h"


static int _initialized = FALSE;

/**
 * Acquire a Zipf random variable.
 *
 * @param alpha - rate of decay (exponent of the denominator must be greater than 0)
 * @param n - range (positive integers)
 * @return a Zipf random variable
 */
int zipf(double alpha, int n) {
  static int    first = TRUE; // Static first time flag
  static double c     = 0;    // Normalization constant
  double        z;            // Uniform random number (0 < z < 1)
  double        sum_prob;     // Sum of probabilities
  double        zipf_value;   // Computed exponential value to be returned
  int           i;            // Loop counter

  if (_initialized == FALSE) rand_val(seedlessRandom());

  // Compute normalization constant on first call only
  if (first == TRUE) {
    for (i=1; i<=n; i++) c = c + (1.0 / pow((double) i, alpha));
    c = 1.0 / c;
    first = FALSE;
  }

  // Pull a uniform random number (0 < z < 1)
  do {
    z = rand_val(0);
  } while ((z == 0) || (z == 1));

  // Map z to the value
  sum_prob = 0;
  for (i=1; i<=n; i++) {
    sum_prob = sum_prob + c / pow((double) i, alpha);
    if (sum_prob >= z) {
      zipf_value = i;
      break;
    }
  }

  // Assert that zipf_value is between 1 and N
  assert((zipf_value >=1) && (zipf_value <= n));

  return(zipf_value);
}

/**
 * Jain's random number generator.
 *
 * @param seed - seed
 * @return a random value between 0.0 and 1.0
 */
double rand_val(int seed) {
  const long  a =      16807; // Multiplier
  const long  m = 2147483647; // Modulus
  const long  q =     127773; // m div a
  const long  r =       2836; // m mod a
  static long x;              // Random int value
  long        x_div_q;        // x divided by q
  long        x_mod_q;        // x modulo q
  long        x_new;          // New x value

  // Set the seed if argument is non-zero and then return zero
  if (seed > 0) {
    _initialized = TRUE;
    x = seed;
    return(0.0);
  }

  // RNG using integer arithmetic
  x_div_q = x / q;
  x_mod_q = x % q;
  x_new = (a * x_mod_q) - (r * x_div_q);
  if (x_new > 0) x = x_new;
  else x = x_new + m;

  return((double) x / m);
}

/**
 * Acquires the harmonic number at n.
 *
 * @param n - a number from 1 to infinity
 * @param alpha - zipfian rate of decay (exponent)
 * @return the n-th harmonic number
 */
double harmonic(int n, double alpha) {
  double sum = 0.0;
  for (long i = 1; i <= n; i++) {
    sum = sum + (1.0 / (pow(i, alpha)));
  }
  return sum;
}

/**
 * Acquires the number of elements in a Zipfian distribution of size N at a target Zipfian CDF.
 * This number is symbolized by the following formula:
 * count = e^((CDF * harmonic number at n) - Euler's Constant) - .5
 *
 * @param n - number of elements in the Zipfian distribution
 * @param alpha - zipfian rate of decay (exponent)
 * @param cdf - target Zipfian CDF (between 0 and 1)
 * @return number of elements at the target CDF for the given size Zipfian distribution
 */
long getZipfCountAtCDF(long n, double alpha, float cdf) {
  return pow(E_NUM, ((cdf * harmonic(n, alpha)) - GAMMA)) - 0.5;
}

/**
 * Acquires the number of levels in a binary search tree for a given number of elements.
 *
 * @param elements - given number of elements
 * @return number of level in a binary search tree
 */
long getNumberOfLevels(long elements) {
  return ceill(log2(elements));
}
