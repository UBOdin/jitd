#ifndef ZIPF_LIB_H_SHIELD
#define ZIPF_LIB_H_SHIELD

#include <math.h>

#define FALSE 0 // Boolean false
#define TRUE  1 // Boolean true
#define E_NUM   2.7182818284590452354 // e
#define GAMMA 0.57721566490153286060651209008240243 // Euler–Mascheroni Constant


/**
 * Acquire a Zipf random variable.
 *
 * @param alpha - rate of decay (exponent of the denominator must be greater than 0)
 * @param n - range (positive integers)
 * @return a Zipf random variable
 */
int zipf(double alpha, int n);

/**
 * Jain's random number generator.
 *
 * @param seed - seed
 * @return a random value between 0.0 and 1.0
 */
double rand_val(int seed);

/**
 * Acquires the harmonic number at n.
 *
 * @param n - a number from 1 to infinity
 * @param alpha - zipfian rate of decay (exponent)
 * @return the n-th harmonic number
 */
double harmonic(int n, double alpha);

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
long getZipfCountAtCDF(long n, double alpha, float cdf);

/**
 * Acquires the number of levels in a binary search tree for a given number of elements.
 *
 * @param elements - given number of elements
 * @return number of level in a binary search tree
 */
long getNumberOfLevels(long elements);

#endif
