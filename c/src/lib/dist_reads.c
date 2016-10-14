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

static long long int splayCount;

void free_workload_test(workload_test *w) { 
  free(w); 
}

struct workload_test *make_workload_test(
    workload_type type,
    bool rebalance, 
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

struct cog *test_reads(struct cog *cog, struct workload_test *w)
{
  struct cog  *cog_return;
  workload_type type = w->type;
  struct timeval stop, start;

  gettimeofday(&start, NULL);
  if (type == RANDOM)
  {
    cog_return = randomreads_on_cog(cog, w);
  }
  else if (type == ZIPFIAN)
  {
    cog_return = zipfianreads_on_cog(cog, w);
  }
  else if (type == HEAVYHITTER)
  {
    cog_return = heavyhitreads_on_cog(cog, w);
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
  return cog_return;
}

struct cog *randomreads_on_cog(struct cog *cog, struct workload_test *w)
{
  printf("Testing JITD performance with random reads ");
  if (w->rebalance) printf("with rebalancing ");
  else printf("without rebalancing ");
  printf("on cog size of %d while performing ", cog_length(cog));
  printf("%ld reads\n", w->number_of_reads);
  printf("Range value is %ld\n", w->range);

  long number = w->number_of_reads;
  long range = w->range;
  bool rebalance = w->rebalance;

  for (int i = 0; i < number; i++) {
    long a = rand() % range;
    long b = rand() % range;
    long low = a <= b ? a : b;
    long high = a > b ? a : b;
    cog = crack_scan(cog, low, high, rebalance);
  }
  return cog;
}

struct cog *zipfianreads_on_cog(struct cog *cog, struct workload_test *w) 
{
  printf("Testing JITD performance with zipfian reads ");
  if (w->rebalance) printf("with rebalancing ");
  else printf("without rebalancing ");
  printf("on cog size of %d while performing ", cog_length(cog));
  printf("%ld reads\n", w->number_of_reads);
  printf("Range value is %ld\n", w->range);

  long number = w->number_of_reads;
  long range = w->range;
  bool rebalance = w->rebalance;
  float alpha = 0.99;
  int n=KEY_RANGE;
  int zipf_rv;
  rand_val(1400);

  for (int i = 1; i < number; i++) {
    zipf_rv = zipf(alpha, n);
    cog = crack_scan(cog, zipf_rv, zipf_rv + range, rebalance);
  }
  return cog;
}

struct cog *heavyhitreads_on_cog(struct cog *cog, struct workload_test *w)
{
  printf("Testing JITD performance with heavyhitter reads ");
  if (w->rebalance) printf("with rebalancing ");
  else printf("without rebalancing ");
  printf("on cog size of %d while performing ", cog_length(cog));
  printf("%ld reads\n", w->number_of_reads);
  printf("Range value is %ld\n", w->range);
  printf("With a shift of %d\n", w->heavy->key_shift);

  struct heavyhit *heavy = w->heavy;
  bool rebalance = w->rebalance;
  long number = w->number_of_reads;
  long range = w->range;
  int key_shift = heavy->key_shift;
  int mod_value = heavy->upper_bound;
  int heavy_value;
  rand_val(1400);

  for (int i = 1; i < number; i++) {
    heavy_value = (next_value(heavy) + key_shift) % mod_value;
    cog = crack_scan(cog, heavy_value, heavy_value + range, rebalance);
    //printf("Heavy hit gave out: %d\n", heavy_value);
  }
  return cog;
}

struct cog *getmedian_policy(struct cog *cog, bool rebalance, int i)
{
  struct cog *cog_median;
  if(rebalance && i > 1000 && i%(twoPow(splayCount)) == 0) {
    cog_median = getMedian(cog);
    cog = splay(cog, cog_median);
    splayCount++;
    //printf("splayCount is now %lld\n", splayCount);
  }
  return cog;
}

struct cog *splay_once(struct cog *cog, int i)
{
  struct cog *cog_median;
  if (i == 1000) {
    cog_median = getMedian(cog);
    cog = splay(cog, cog_median);
  } else {
    return cog;
  }
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
