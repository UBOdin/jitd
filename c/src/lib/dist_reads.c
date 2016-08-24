#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "cog.h"
#include "cracker.h"
#include "splay.h"
#include "util.h"
#include "zipf.h"
#include "heavyhit.h"
#include "dist_reads.h"

#define BUFFER_SIZE 10
#define KEY_RANGE 1000000

void free_workload_test(workload_test *w) { 
  cleanup(w->cog);
  free(w); 
}

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
  w->test_array_size = test_array_size;
  w->number_of_reads = number_of_reads;
  w->range = range;
  return w;
}

struct cog *testReads(struct workload_test *w)
{
  struct cog *cog, *cog_return;
  cog = w->cog;
  workload_type type = w->type;
  bool rebalance = w->rebalance;
  long number_of_reads = w->number_of_reads;
  long range = w->range;
  long arraySize = w->test_array_size;
  struct timeval stop, start;

  gettimeofday(&start, NULL);
  if (type == RANDOM)
  {
    cog_return = randomread_randomarray(cog, rebalance, number_of_reads, range);
  }
  else if (type == ZIPFIAN)
  {
    cog_return = zipfianread_randomarray(cog, rebalance, number_of_reads, range);
  }
  else if (type == HEAVYHITTER)
  {
    cog_return = heavyhitread_randomarray(cog, rebalance, number_of_reads, range);
  }
  else
  {
    printf("Invalid workload type, please specify RANDOM, ZIPFIAN, ");
    printf("or HEAVYHITTER, program now terminating\n");
    exit(0);
  }
  gettimeofday(&stop, NULL);

  long long startms = start.tv_sec * 1000LL + start.tv_usec / 1000;
  long long stopms = stop.tv_sec * 1000LL + stop.tv_usec / 1000;
  printf("Took %lld milliseconds\n", stopms - startms);
  w->cog = cog_return;
  return cog_return;
}

struct cog *randomread_randomarray(struct cog *cog, bool rebalance, 
    long number, long range)
{
  printf("Testing JITD performance on random array with random reads ");
  if (rebalance) printf("with rebalancing ");
  else printf("without rebalancing ");
  printf("on array size of %d while performing ", cog_length(cog));
  printf("%ld reads\n", number);

  printf("For range value %ld: ", range);
  cog = randomReads(cog, number, range);
  if (rebalance) { splay(cog, getMedian(cog)); }
  return cog;
}

struct cog *zipfianread_randomarray(struct cog *cog, bool rebalance, 
    long number, long range) 
{
  printf("Testing JITD performance on random array with zipfian reads ");
  if (rebalance) printf("with rebalancing ");
  else printf("without rebalancing ");
  printf("on array size of %d while performing ", cog_length(cog));
  printf("%ld reads\n", number);
  printf("For range value %ld: ", range);

  float alpha =0.99;
  int n=KEY_RANGE;
  int zipf_rv;
  int splayCount = 0;
  rand_val(1400);
  struct cog *cog_median;

  for (int i=1; i<number; i++) {
    zipf_rv = zipf(alpha, n);
    cog = crack_scan(cog, zipf_rv, zipf_rv + range);
    if(rebalance && i > 1000 && i%(twoPow(splayCount)) == 0) {
      cog_median = getMedian(cog);
      cog = splay(cog, cog_median);
      splayCount++;
    }
    // Use this if statement instead of one above for really slow performance
    //if(rebalance && (i > 100 || i%2 == 0)) {
    //  cog_median = getMedian(cog);
    //  cog = splay(cog, cog_median);
    //}
  }
  return cog;
}

struct cog *heavyhitread_randomarray(struct cog *cog, bool rebalance, 
    long number, long range) 
{
  //float alpha = 0.99;
  //int n = KEY_RANGE;
  //int heavy_rv;
  //rand_val(1400);
  //struct cog *cog_median;

  //for (int i=1; i<number; i++) 
  //{
  //  heavy_rv = heavy(alpha, n);
  //  cog = crack_scan(cog, heavy_rv, heavy_rv + range);
  //  if(i > 1000 || i%2 == 0) 
  //  {
  //    cog_median = getMedian(cog);
  //    cog = splay(cog, cog_median);
  //  }
  //}
  return cog;
}

buffer mk_random_buffer(int size) 
{
  buffer b = buffer_alloc(size);
  int i;
  for(i = 0; i < size; i++)
  {
    b->data[i].key = rand() % KEY_RANGE;
    b->data[i].value = rand();
  }
  //record_dump(b->data, 0, size); // Useful for viewing test1
  return b;
}

buffer mk_sorted_buffer(int size) 
{
  buffer b = mk_random_buffer(size);
  record_sort(b->data, 0, size);
  record_dump(b->data, 0, size);
  return b;
}

cog *mk_random_array(int size) 
{
  return make_array(0, size, mk_random_buffer(size));
}

cog *mk_sorted_array(int size) 
{
  return make_sortedarray(0, size, mk_sorted_buffer(size));
}

void test_scan(cog *c, long low, long high) 
{
  iterator iter = scan(c, low, high);
  iter_dump(iter);
  iter_cleanup(iter);
}

long long int twoPow(long long int exp)
{
  long long int base = 2;
  long long int result = 1;
  while (exp) {
    if (exp & 1) result *= base;
    exp >>= 1;
    base *= base;
  }
  return result;
}
