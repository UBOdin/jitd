#ifndef CRACKER_H_SHEILD
#define CRACKER_H_SHEILD

cog *pushdown_concats(cog *c, long low, long high);

cog *crack_scan(cog *c, long low, long high);

cog *crack_one(cog *c, long val);

cog *crack(cog *c, long low, long high);

#endif //CRACKER_H_SHEILD
