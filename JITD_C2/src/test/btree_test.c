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
#include "dist_reads.h"

#define BUFFER_SIZE 10
#define KEY_RANGE 1000000

void test1() 
{
  // Test for making a random array cog
  printf("test 1\n");
  cog *c = mk_random_array(BUFFER_SIZE);
  test_scan(c, 200, 700);
  cleanup(c);
  printf("\n");
}

void test2() 
{
  // Test for making a sorted array cog
  printf("test 2\n");
  cog *c = mk_sorted_array(BUFFER_SIZE);
  test_scan(c, 200, 700);
  cleanup(c);
  printf("\n");
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
  printf("\n");
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
  printf("\n");
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
  printf("\n");
}

void test6()
{
  // Perform pre-loaded tests for tree operations on cracking an array
  printf("test 6\n");
  testTreeOnArrayCrack(false, 1000000, 10000);
}

void test7()
{
  // Perform pre-loaded tests for tree operations on cracking an array
  printf("test 7\n");
  testTreeOnArrayCrack(true, 1000000, 10000);
}

/*|__testTreeOnArrayCrack__|*/
void testTreeOnArrayCrack(bool rebalance, int arraySize, int reads)
{
  if (rebalance)
  {printf("Testing JITD performance on random array with random reads ");
   printf("without rebalancing ");}
  else
  {printf("Testing JITD performance on random array with random reads ");
   printf("with rebalancing ");}
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
  printf("\n");
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

void test8()
{
  printf("test 8\n");
  testZipfianNoSplay(10000, 1000);
  testZipfianWithSplay(10000, 1000);
  printf("\n");
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

struct cog *execute_workload_test(struct workload_test *w)
{
  struct cog *cog;
  cog = mk_random_array(w->test_array_size);
  w->cog = cog;
  cog = testReads(w);
  /* remember to free workload when done */
  return cog;
}

void test9()
{
  printf("test 9\n");
  struct workload_test *work;
  struct cog *cog;
  work = make_workload_test(RANDOM, true, 1000000, 10000, 10000);
  cog = execute_workload_test(work);
  printf("\n");
}

int main(int argc, char **argv) 
{
  int rand_start = 42; //time(NULL)
  srand(rand_start);
  test1();
  srand(rand_start);
  test2();
  srand(rand_start);
  test3();
  srand(rand_start);
  test4();
  srand(rand_start);
  test5();
  srand(rand_start);
  test6();
  srand(rand_start);
  test7();
  srand(rand_start);
  test8();
  srand(rand_start);
  test9();
  //testHeavyHitterNoSplay(10000);
  //testHeavyHitterWithSplay(10000);
}
