#ifndef DIST_READS_H_SHEILD
#define DIST_READS_H_SHEILD

#include <stdbool.h>

typedef enum 
{
  RANDOM, ZIPFIAN, HEAVYHITTER
} workload_type;

typedef struct workload_test
{
  workload_type type;
  bool rebalance;
  long test_array_size;
  long number_of_reads;
  long range;
} workload_test;

struct cog *testReads(struct cog *(*function)(struct workload_test *));

struct cog *doZipfianReads(struct cog *cog, long number, long range);

struct cog *zipfianReads_splay(struct cog *cog, long number, long range);

struct cog *heavyHitterNoSplay();

struct cog *heavyHitterWithSplay();

#endif //DIST_READS_H_SHEILD
