#ifndef CRACKER_H_SHEILD
#define CRACKER_H_SHEILD

cog *pushdown_concats(cog *c, long low, long high);

cog *crack_scan(cog *c, long low, long high);

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

#endif //CRACKER_H_SHEILD
