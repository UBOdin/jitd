#ifndef CRACKER_H_SHEILD
#define CRACKER_H_SHEILD

#include <stdbool.h>

cog *pushdown_concats(cog *c, long low, long high);

//cog *crack_scan(cog *c, long low, long high);

cog *crack_scan(cog *c, long low, long high, bool rebalance);

#ifdef __HARVEST

#define FALSE 0 // Boolean false
#define TRUE  1 // Boolean true

/**
 * Acquires the last BTree cog that was read from in crack_scan().
 *
 * @return the last BTree cog that was read from in crack_scan().
 */
struct cog *getHarvest();
#endif

cog *crack_one(cog *c, long val);

cog *crack(cog *c, long low, long high);

struct cog *pivot_if_needed(bool rebalance, struct cog *c);

bool offbalance_exist(struct cog *c);

bool pivot_advantage(struct cog *c);

struct cog *pivot(struct cog *c);

#endif //CRACKER_H_SHEILD
