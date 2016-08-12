#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>

#include "cog.h"
#include "cracker.h"
#include "splay.h"
#include "util.h"
#include "adaptive_merge.h"
#include "zipf.h"
#include "policy.h"

#define BUFFER_SIZE 10
#define KEY_RANGE 1000000

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

void test1() 
{
  // Test for making a random array cog
  printf("test 1\n");
  cog *c = mk_random_array(BUFFER_SIZE);
  test_scan(c, 200, 700);
  cleanup(c);
}

void test2() 
{
  // Test for making a sorted array cog
  printf("test 2\n");
  cog *c = mk_sorted_array(BUFFER_SIZE);
  test_scan(c, 200, 700);
  cleanup(c);
}

void test3() 
{
  // Test for making a concat cog and doing a scan for it
  printf("test 3\n");
  cog *c = make_concat(
      mk_random_array(BUFFER_SIZE),
      mk_random_array(BUFFER_SIZE)
    );
  test_scan(c, 100, 200);
  cleanup(c);
}

/*|__test4__|*/
void test4() 
{
  printf("test 4\n");
  cog *c = make_concat(
      mk_random_array(BUFFER_SIZE),
      mk_random_array(BUFFER_SIZE)
    );
  c = crack(c, 100, 200);
  c = crack(c, 400, 700);
  c = crack(c, 800, 900);
  test_scan(c, 1, 1000);
  cleanup(c);
}

/*|__test5__|*/
void test5() 
{
  // Perform the test for adaptive merge
  printf("test 5\n");
  cog *c = make_concat(
      mk_random_array(BUFFER_SIZE),
      mk_random_array(BUFFER_SIZE)
      );
  double_struct *ret;
  printf("Scan 1\n");
  ret = amerge(c, 100, 200);
  iter_dump(ret->iter);
  iter_cleanup(ret->iter);
  c = ret->cog;
  free(ret);
  printf("Scan 2\n");
  ret = amerge(c, 300, 700);
  iter_dump(ret->iter);
  cleanup(ret->cog);
  iter_cleanup(ret->iter);
  free(ret);
}

/*|__testTreeOnArrayCrack__|*/
void testTreeOnArrayCrack(bool rebalance, int arraySize, int reads)
{
  if (rebalance)
  {printf("Test JITD performance on random array without rebalancing ");}
  else
  {printf("Test JITD performance on random array with rebalancing ");}
  printf("on array size of %d while performing ", arraySize);
  printf("%d reads\n", reads);

  struct cog *cog;
  cog = mk_random_array(arraySize);
  int rangeValues[6] = {10000, 250, 1000, 5000, 10, 5000};

  for (int i=0; i<6; i++)
  {
    printf("For range value %d: ", rangeValues[i]);
    cog = timeRun(randomReads, cog, reads, rangeValues[i]);
    if (rebalance) { splay(cog, getMedian(cog)); }
  }
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
    //printf("%d \n", zipf_rv);
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
    //printf("%d \n", zipf_rv);
  }
  return cog;
}

/*|__testZipfianNoSplay__|*/
void testZipfianNoSplay(int arraySize, int reads) 
{
  printf("Running Zipfian test with no splaying\n");
  struct cog *cog, *cog_result;
  cog = mk_random_array(arraySize);
  cog_result = timeRun(doZipfianReads, cog, reads, 1000);
}

/*|__testZipfianWithSplay__|*/
void testZipfianWithSplay(int arraySize, int reads)
{
  printf("Running Zipfian test with splaying\n");
  struct cog *cog, *cog_result;
  cog = mk_random_array(arraySize);
  cog_result = timeRun(zipfianReads_splay, cog, reads, 1000);
}

/*|__testHeavyHitterNoSplay__|*/
void testHeavyHitterNoSplay(int reads)
{
  printf("Running HeavyHitter test with no splaying\n");
}

/*|__testHeavyHitterWithSplay__|*/
void testHeavyHitterWithSplay(int reads)
{
  printf("Running HeavyHitter test with splaying\n");
}

int main(int argc, char **argv) 
{
  int rand_start = 42; //time(NULL)
  srand(rand_start);
  //test1();
  //srand(rand_start);
  //test2();
  //srand(rand_start);
  //test3();
  //srand(rand_start);
  //test4();
  //srand(rand_start);
  //test5();
  //srand(rand_start);
  bool rebalance;
  testTreeOnArrayCrack(rebalance = true, 1000000, 10000);
  testTreeOnArrayCrack(rebalance = false, 1000000, 10000);
  //testZipfianNoSplay(10000, 1000);
  //testZipfianWithSplay(10000, 1000);
  //testHeavyHitterNoSplay(10000);
  //testHeavyHitterWithSplay(10000);
}
