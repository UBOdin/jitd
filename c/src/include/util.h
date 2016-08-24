#ifndef UTIL_LIB_H_SHIELD
#define UTIL_LIB_H_SHIELD

#include "cog.h"


/**
 * Prints the internal representation of the JITD providing a detailed layout
 * of the current cogs and data present within.
 *
 * @param cog - the root cog
 * @param depth - depth of the current cog in the tree - set to 0 for root
 */
void printJITD(struct cog *c, int depth);

#ifdef __ADVANCED
/**
 * Converts the JITD to JSON and places it in the file './test.txt'.
 *
 * @param cog - the root cog
 * @param name - output file name
 */
void jsonJITD(struct cog *cog, char *name);
#endif

/** Prints the current pre-processor mode. */
void printMode();

/**
 * Acquires the count of BTree nodes in a tree.
 *
 * @param cog - the root BTree node in the tree
 * @return the count of BTree nodes in the tree
 */
long getBtreeNodeCount(struct cog *cog);

/**
 * Creates an in-order list of BTree Nodes for a given tree.
 * NOTE: the in-order list is allocated with malloc, so deallocate with free when done!
 *
 * @param cog - root BTree cog
 * @param count - the count of BTree nodes in the tree
 * @return the in-order list
 */
struct cog **inorder(struct cog *cog, long count);

/**
 * Acquires the median BTree node in a JITD.
 *
 * @param root - the root node of a JITD
 * @return the median node of a JITD
 */
struct cog *getMedian(struct cog *root);

/**
 * Executes a given function and times the execution.
 *
 * @param function - Function to run
 * @param cog - cog parameter for function
 * @param a - first long parameter for function
 * @param b - second long parameter for function
 * @return the resulting BTree
 */
struct cog *timeRun(struct cog *(*function)(struct cog *, long, long),
                    struct cog *cog,
                    long a,
                    long b);

/**
 * Do a given number of random reads on a cog.
 *
 * @param cog - the given cog
 * @param number - number of reads to do on a cog
 * @param range - the key range for reads
 * @return the resulting BTree
 */
struct cog *randomReads(struct cog *cog, long number, long range);

/**
 * Do a given number of zipfian reads on a cog.
 *
 * @param cog - the given cog
 * @param number - number of reads to do on a cog
 * @param alpha - zipfian rate of decay
 * @param range - the key range for reads
 * @return the resulting BTree
 */
struct cog *zipfianReads(struct cog *cog, double alpha, long number, long range);

/**
 * Acquire a random number - no seed issues.
 *
 * @return a random number
 */
int seedlessRandom();

/**
 * Acquire a random array cog.
 *
 * @param size - size of the array
 * @param range - key range
 * @return a random array cog
 */
cog *getRandomArray(int size, int range);

#ifdef __HARVEST
/**
 * Run a test involving reads and splaying on a harvested value (last value read).
 *
 * @param cog - the root of the JITD
 * @param reads - number of reads per step
 * @param range - the key range for reads
 * @param doSplay - boolean TRUE or FALSE, if TRUE splay after every step, otherwise just read
 * @param steps - number of steps
 * @return the root of the resulting JITD
 */
struct cog *splayOnHarvest(struct cog *cog, long reads, long range, int doSplay, int steps);
#endif

#ifdef __ADVANCED
/**
 * Acquires the cumulative reads at a node if possible.
 *
 * @param cog - a given cog
 * @return the cumulative reads at the cog, 0 if it is NULL or it is not a BTree cog
 */
long getCumulativeReads(struct cog *cog);

/**
 * Acquires the actual read count at a given BTree node.
 *
 * @param cog - a BTree node
 * @return the actual read count for that given BTree node
 */
long getReadsAtNode(struct cog *cog);
#endif

#endif
