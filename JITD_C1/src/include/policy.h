#ifndef POLICY_LIB_H_SHIELD
#define POLICY_LIB_H_SHIELD

#ifdef __ADVANCED
#include "cog.h"


/**
 * Moves up nodes into the given number of levels so that the resulting tree has close to a
 * Zipfian distribution for the given levels based on the number of reads.
 *
 * @param cog - root of the tree
 * @param levels - given number of levels
 * @return the new root of the rearranged tree
 */
struct cog *zipfinize(struct cog *cog, long levels);

/**
 * Decays the tree read counts when necessary so that they don't overflow.
 *
 * @param cog - given tree
 * @return root of the given tree
 */
struct cog *decay(struct cog *cog);

/**
 * Acquires the current interval for running the policy.  Self adjusting based on the threshold.
 *
 * @return the next interval
 */
long getInterval();

/**
 * Sets the policy interval threshold.
 *
 * @param - the policy interval
 */
void setThreshold(long threshold);

/**
 * Initializes the interval for running the policy.
 *
 * @param interval - initial interval for the policy
 * @param threshold - number of moves signifying a need to change the interval
 */
void initInterval(long interval, long threshold);

/**
 * Sets the policy interval.
 *
 * @param - the policy interval
 */
void setInterval(long interval);

/**
 * Acquires the policy interval threshold.
 *
 * @return the policy interval threshold
 */
long getThreshold();

/**
 * Acquires the number of splays performed during the last zipfinize operation.
 *
 * @return the number of splays performed during the last zipfinize operation
 */
long getSplays();
#endif
#endif
