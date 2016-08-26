#ifndef DIST_READS_H_SHEILD
#define DIST_READS_H_SHEILD

#include <stdbool.h>
#include "heavyhit.h"

typedef enum 
{
  RANDOM, ZIPFIAN, HEAVYHITTER
} workload_type;

typedef struct workload_test
{
  workload_type type;
  struct zipfian *zipfian;
  struct heavyhit *heavy;
  bool rebalance;
  long number_of_reads;
  long range;
} workload_test;

struct workload_test *make_workload_test(
    workload_type type,
    bool rebalance, 
    long number_of_reads, 
    long range
    );

struct cog *test_reads(struct cog *cog, struct workload_test *w);

void testTreeOnArrayCrack(bool rebalance, int arraySize, int reads);

struct cog *randomreads_on_cog(struct cog *cog, struct workload_test *w);

struct cog *zipfianreads_on_cog(struct cog *cog, struct workload_test *w);

struct cog *heavyhitreads_on_cog(struct cog *cog, struct workload_test *w);

struct cog *getmed_policy(struct cog *cog, bool rebalance, int i);

buffer mk_random_buffer(int size);

buffer mk_sorted_buffer(int size);

cog *mk_random_array(int size);

cog *mk_sorted_array(int size);

void test_scan(cog *c, long low, long high);

long long int twoPow(long long int exp);

void free_workload_test(workload_test *w);

#endif //DIST_READS_H_SHEILD
