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
  struct cog *cog;
  workload_type type;
  bool rebalance;
  long test_array_size;
  long number_of_reads;
  long range;
} workload_test;

struct workload_test *make_workload_test(
    workload_type type,
    bool rebalance, 
    long test_array_size,
    long number_of_reads, 
    long range
    );

struct cog *testReads(struct workload_test *w);

void testTreeOnArrayCrack(bool rebalance, int arraySize, int reads);

void free_workload_test(workload_test *w);

struct cog *execute_workload_test(struct workload_test *w);

struct cog *zipfianread_randomarray(struct cog *cog, bool rebalance, 
    long number, long range);

struct cog *randomread_randomarray(struct cog *cog, bool rebalance, 
    long number, long range);

struct cog *heavyhitread_randomarray(struct cog *cog, bool rebalance, 
    long number, long range);

struct cog *heavyhit_test(bool rebalance, struct cog *cog, 
    struct heavyhit *heavy);

buffer mk_random_buffer(int size);

buffer mk_sorted_buffer(int size);

cog *mk_random_array(int size);

cog *mk_sorted_array(int size);

void test_scan(cog *c, long low, long high);

long long int twoPow(long long int exp);

#endif //DIST_READS_H_SHEILD
