//#ifdef __ADVANCED
#include <stdio.h>
#include <limits.h>

#include "cog.h"
#include "policy.h"
#include "splay.h"
#include "util.h"


#define _DECAY_FACTOR    3
#define _DECAY_THRESHOLD LONG_MAX - (LONG_MAX / _DECAY_FACTOR)

static long _interval  = 100;
static long _threshold = 10;
static long _splays    = 0;

/**
 * Sets the policy interval.
 *
 * @param - the policy interval
 */
void setInterval(long interval) {
  _interval = interval;
}

/**
 * Sets the policy interval threshold.
 *
 * @param - the policy interval
 */
void setThreshold(long threshold) {
  _threshold = threshold;
}

/**
 * Initializes the interval for running the policy.
 *
 * @param interval - initial interval for the policy
 * @param threshold - number of moves signifying a need to change the interval
 */
void initInterval(long interval, long threshold) {
  setInterval(interval);
  setThreshold(threshold);
}

/**
 * Acquires the current interval for running the policy.  Self adjusting based on the threshold.
 *
 * @return the next interval
 */
long getInterval() {
  return _interval;
}

/**
 * Acquires the policy interval threshold.
 *
 * @return the policy interval threshold
 */
long getThreshold() {
  return _threshold;
}

/**
 * Acquires the number of splays performed during the last zipfinize operation.
 *
 * @return the number of splays performed during the last zipfinize operation
 */
long getSplays() {
  return _splays;
}
//#endif
