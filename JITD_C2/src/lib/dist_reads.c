#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "cog.h"
#include "cracker.h"
#include "splay.h"
#include "util.h"
#include "zipf.h"
#include "dist_reads.h"

#define BUFFER_SIZE 10
#define KEY_RANGE 1000000

void free_workload_test(workload_test *w) { free(w); }

struct workload_test *make_workload_test(
    workload_type type,
    bool rebalance, 
    long test_array_size,
    long number_of_reads, 
    long range)
{
  workload_test *w = malloc(sizeof(struct workload_test));
  w->type = type;
  w->rebalance = rebalance;
  w->number_of_reads = number_of_reads;
  w->range = range;
  return w;
}

//struct workload_test *make_random_workload_test(
//    bool rebalance, 
//    long test_array_size,
//    long number_of_reads, 
//    long range)
//{
//  workload_test *w = malloc(sizeof(struct workload_test));
//  w->type = RANDOM;
//  w->rebalance = rebalance;
//  w->number_of_reads = number_of_reads;
//  w->range = range;
//  return w;
//}
//
//struct workload_test *make_zipfian_workload_test(
//    bool rebalance, 
//    long test_array_size,
//    long number_of_reads, 
//    long range)
//{
//  workload_test *w = malloc(sizeof(struct workload_test));
//  w->type = ZIPFIAN;
//  w->rebalance = rebalance;
//  w->number_of_reads = number_of_reads;
//  w->range = range;
//  return w;
//}
//
//struct workload_test *make_heavyhitter_workload_test(
//    bool rebalance, 
//    long test_array_size,
//    long number_of_reads, 
//    long range)
//{
//  workload_test *w = malloc(sizeof(struct workload_test));
//  w->type = HEAVYHITTER;
//  w->rebalance = rebalance;
//  w->number_of_reads = number_of_reads;
//  w->range = range;
//  return w;
//}

struct cog *testReads(struct cog *(*function)(struct workload_test *),
                      struct workload_test *workload_test)
{
  struct timeval stop, start;
  gettimeofday(&start, NULL);
  struct cog *out = (*function)(workload_test);
  gettimeofday(&stop, NULL);
  long long startms = start.tv_sec * 1000LL + start.tv_usec / 1000;
  long long stopms = stop.tv_sec * 1000LL + stop.tv_usec / 1000;
  printf("Took %lld milliseconds\n", stopms - startms);
  return out;
}



/**
 * Function to generate the zipfian reads calling the zipf function.
 *
 * @param cog - the given cog
 * @param number - the number of scans to be performed on the given cog
 * @param range - the range of number to be scannned(selectivity)
 * @return resulting JITD
 */
struct cog *doZipfianReads(struct cog *cog, long number, long range) 
{
  float alpha =0.99;
  int n=KEY_RANGE;
  int zipf_rv;
  rand_val(1400);

  for (int i=1; i<number; i++) 
  {
    zipf_rv = zipf(alpha, n);
    cog = crack_scan(cog, zipf_rv, zipf_rv + range);
  }
  return cog;
}

/**
 * Function to perform zipfian read with splay operation.
 *
 * @param cog - the given cog
 * @param number - the number of iterations
 * @param range - the selectivity range
 * @return resulting JITD
 */
struct cog *zipfianReads_splay(struct cog *cog, long number, long range) 
{
  float alpha = 0.99;
  int n = KEY_RANGE;
  int zipf_rv;
  rand_val(1400);
  struct cog *cog_median;

  for (int i=1; i<number; i++) 
  {
    zipf_rv = zipf(alpha, n);
    cog = crack_scan(cog, zipf_rv, zipf_rv + range);
    if(i > 100 || i%2 == 0) 
    {
      cog_median = getMedian(cog);
      cog = splay(cog, cog_median);
    }
  }
  return cog;
}

