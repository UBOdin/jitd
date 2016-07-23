#ifdef __ADVANCED
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
 * Attempt to find a good splay candidate - usually highest node in the subtrees.
 *
 * @param cog - root cog used for search
 * @param reads - number of reads to top
 * @return a good splay candidate, or NULL if none such
 */
struct cog *findSplayCandidate(struct cog *cog, long reads) {
  struct cog *candidate = NULL;
  struct cog *contender = NULL;
  if (cog == NULL || cog->type != COG_BTREE) return NULL;
  if (cog->data.btree.rds <= reads) return NULL;

  long cogReads = getReadsAtNode(cog);
  if (cogReads > reads) {
    contender = cog;
    reads = cogReads;
  }

  candidate = findSplayCandidate(cog->data.btree.lhs, reads);
  if (candidate != NULL) return candidate;
  candidate = findSplayCandidate(cog->data.btree.rhs, reads);
  if (candidate != NULL) return candidate;
  return contender;
}

/**
 * Zipfinizes the subtree.
 *
 * @param cog - root of the tree
 * @param levels - given number of levels
 * @return the new root of the rearranged tree
 */
struct cog *zipfinizeSubtree(struct cog *cog, long levels) {
  if (levels == 0 || cog == NULL || cog->type != COG_BTREE) return cog;
  long reads = getReadsAtNode(cog);
  struct cog *candidate = findSplayCandidate(cog, reads);

  struct cog *rearranged;
  if (candidate == NULL) {
    rearranged = cog;
  } else {
    rearranged = splay(cog, candidate);
    _splays += 1;
  }

  long remaining = levels - 1;
  rearranged->data.btree.lhs = zipfinizeSubtree(rearranged->data.btree.lhs, remaining);
  rearranged->data.btree.rhs = zipfinizeSubtree(rearranged->data.btree.rhs, remaining);
  return rearranged;
}

/**
 * Moves up nodes into the given number of levels so that the resulting tree has close to a
 * Zipfian distribution for the given levels based on the number of reads.
 *
 * @param cog - root of the tree
 * @param levels - given number of levels
 * @return the new root of the rearranged tree
 */
struct cog *zipfinize(struct cog *cog, long levels) {
  decay(cog);
  _splays = 0;
  struct cog *rearranged = zipfinizeSubtree(cog, levels);
  double speed = 0.1;
  double delta = speed * (double) _interval;
  if (_splays <= _threshold) _interval += delta;
  else _interval -= delta;
  return rearranged;
}

/**
 * Decays the tree by the given factor. For example, if the factor is x the read count for every
 * node in the tree will be reduced as such: floor(reads/x)
 *
 * @param cog - given tree
 */
void decaySubtree(struct cog *cog) {
  if (cog != NULL && cog->type == COG_BTREE) {
    cog->data.btree.rds /= _DECAY_FACTOR;
    decaySubtree(cog->data.btree.lhs);
    decaySubtree(cog->data.btree.rhs);
  }
}

/**
 * Decays the tree read counts when necessary so that they don't overflow.
 *
 * @param cog - given tree
 * @return root of the given tree
 */
struct cog *decay(struct cog *cog) {
  if (cog->data.btree.rds >= _DECAY_THRESHOLD) decaySubtree(cog);
  return cog;
}

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
#endif
