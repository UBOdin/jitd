
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
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
  //record_dump(b->data, 0, size);
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
  struct cog *p =  make_array(0, size, mk_random_buffer(size));
  printf("p=%u \n",p);
  return p;
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
  struct cog *temp;
  rand_val(1400);
  
  for (int i=1; i<number; i++) 
  {
    zipf_rv = zipf(alpha, n);
    //printf("%d \n", zipf_rv);
    //temp = crack_scan(cog,zipf_rv,zipf_rv+range);
    //cleanup(temp);
    //printf("1. %d \n", zipf_rv);
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
  struct cog *temp;
  rand_val(1400);
  struct cog *cog_median;
  
  for (int i=1; i<number; i++) 
  {
    zipf_rv = zipf(alpha, n);
    //if(i%2==0) cog_median = getMedian(cog);
    // temp= crack_scan(cog, zipf_rv, zipf_rv + range);
    //cleanup(temp);
    // if(i > 100) splay(cog, cog_median);
    //printf("%d \n", zipf_rv);
  }
  return cog;
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
  // Test for making a concat cog and doing a scan for
  printf("test 3\n");
  cog *c = 
  make_concat(
    mk_random_array(BUFFER_SIZE),
    mk_random_array(BUFFER_SIZE)
  );
  c = crack(c, 100, 150);
  test_scan(c, 100, 200);
  cleanup(c);
}

//void test4() 
//{
//    printf("test 4\n");
//    cog *c = make_concat(
//        mk_random_array(BUFFER_SIZE),
//        mk_random_array(BUFFER_SIZE)
//        );
//    c = crack(c, 100, 200);
//    c = crack(c, 400, 700);
//    c = crack(c, 800, 900);
//    test_scan(c, 1, 1000);
//    //racleanup(c);
//}

//void test5() 
//{
//  // Perform the test for adaptive merge
//  printf("test 5\n");
//  cog *c = make_concat(
//      mk_random_array(BUFFER_SIZE),
//      mk_random_array(BUFFER_SIZE)
//      );
//  double_struct *ret;
//  printf("Scan 1\n");
//  ret = amerge(c, 100, 200);
//  iter_dump(ret->iter);
//  iter_cleanup(ret->iter);
//  c = ret->cog;
//  free(ret);
//  printf("Scan 2\n");
//  ret = amerge(c, 300, 700);
//  iter_dump(ret->iter);
//  cleanup(ret->cog);
//  iter_cleanup(ret->iter);
//  free(ret);
//}

//void splayTest() 
//{
//  printf("Splaying Test:\n");
//  cog *six = make_btree(NULL, NULL, 6);
//  cog *eight = make_btree(NULL, NULL, 8);
//  cog *seven = make_btree(six, eight, 7);
//  cog *four = make_btree(NULL, NULL, 4);
//  cog *five = make_btree(four, seven, 5);
//  cog *two = make_btree(NULL, NULL, 2);
//  cog *three = make_btree(two, five, 3);
//  cog *ten = make_btree(NULL, NULL, 10);
//  cog *nine = make_btree(three, ten, 9);
//  cog *zero = make_btree(NULL, NULL, 0);
//  cog *one = make_btree(zero, nine, 1);
//  cog *twelve = make_btree(NULL, NULL, 12);
//  cog *eleven = make_btree(one, twelve, 11);
//  printf("Before splay:\n");
//  printJITD(eleven,0);
//  printf("After splay:\n");
//  splay(eleven, seven);
//  printJITD(seven, 0);
//}*/

//void test6(int reads)
//{
//  printf("Test JITD performance without splaying\n");
//  struct cog *cog;
//  cog = mk_random_array(1000000);
//  //cog = timeRun(randomReads, cog, reads, 10000);
//  //cog = timeRun(randomReads, cog, reads, 250);
//  //cog = timeRun(randomReads, cog, reads, 1000);
//  //cog = timeRun(randomReads, cog, reads, 5000);
//  cog = timeRun(randomReads, cog, reads, 10);
//}

//void test7(int reads)
//{
//  printf("Test JITD performance with splaying\n");
//  struct cog *cog;
//  cog = mk_random_array(1000000);
//  cog = timeRun(randomReads, cog, reads, 10000);
//  splay(cog, getMedian(cog));
//  //cog = timeRun(randomReads, cog, reads, 250);
//  //splay(cog, getMedian(cog));
//  //cog = timeRun(randomReads, cog, reads, 1000);
//  //splay(cog, getMedian(cog));
//  //cog = timeRun(randomReads, cog, reads, 5000);
//  //splay(cog, getMedian(cog));
//  //cog = timeRun(randomReads, cog, reads, 10);
//  //splay(cog, getMedian(cog));
//}

//void test8(int reads) 
//{
//  printf("Test zipfian performance without splaying\n");
//  struct cog *cog, *cog_result, *cog_median;
//  cog = mk_random_array(1000000);
//  printf("cog = %u\n",cog);
/  /
//  // Cog without splaying
//  cog_result = timeRun(doZipfianReads, cog, reads, 10000);
//  //cog_result = timeRun(doZipfianReads, cog, reads, 250);
//  //cog_result = timeRun(doZipfianReads, cog, reads, 1000);
//  //cog_result = timeRun(doZipfianReads, cog, reads, 5000);
//  //cog_result = timeRun(doZipfianReads, cog, reads, 10);

//  printf("cog_result=%u\n",cog_result);


//  struct cog **inorder_list=inorder(struct cog *cog);
//  cog_median = getMedian(cog_result);

//  Cog with splaying
//  timeRun(splayTest, cog, 1000, 1000000);
//  printJITD(cog_result, 0);
//}

//void test9(int reads) 
//{
//  printf("Test zipfian performance with splaying\n");
//  struct cog *cog,*cog_result,*cog_median;
//  cog = mk_random_array(1000000);

//  // Cog without splaying
//  cog_result = timeRun(zipfianReads_splay, cog, reads, 10000);
//  struct cog **inorder_list=inorder(struct cog *cog);
//  cog_median = getMedian(cog_result);
//  cog_result = timeRun(zipfianReads_splay, cog, reads, 250);
//  //struct cog **inorder_list=inorder(struct cog *cog);
//  //cog_median = getMedian(cog_result);
//  //cog_result = timeRun(zipfianReads_splay, cog, reads, 1000);
//  //struct cog **inorder_list=inorder(struct cog *cog);
//  //cog_median = getMedian(cog_result);
//  //cog_result = timeRun(zipfianReads_splay, cog, reads, 5000);
//  //struct cog **inorder_list=inorder(struct cog *cog);
//  //cog_median = getMedian(cog_result);

//  //cog_result = timeRun(zipfianReads_splay, cog, reads, 10);
//  //struct cog **inorder_list=inorder(struct cog *cog);
//  cog_median = getMedian(cog_result);


//  // Cog with splaying
//  timeRun(splayTest, cog, 1000, 1000000);
//  printJITD(cog_result, 0);
//}

#ifdef __ADVANCED
/** A simple test that makes sure read counters are properly incremented when upon cracking. */
void readsCounterTest() 
{
  int size = 100;
  buffer b = buffer_alloc(size);

  for(int i = 0; i < size; i++)
  {
    b->data[i].key = rand() % 100;
    b->data[i].value = rand();
  }

  struct cog *cog = make_array(0, size, b);
  cog = crack(cog, 25, 75);
  cog = crack(cog, 75, 85);
  cog = crack(cog, 15, 25);
  cog = crack(cog, 65, 75);
  printJITD(cog, 0);

  cog = splay(cog, cog->data.btree.lhs);
  printf("\n\n");
  printJITD(cog, 0);

  cog = splay(cog, cog->data.btree.rhs->data.btree.rhs);
  printf("\n\n");
  printJITD(cog, 0);
}

void testZipfinize() 
{
  double alpha = 1;
  long elements = 10000;
  long reads = 1000;
  long count = getZipfCountAtCDF(elements, alpha, .5);
  long levels = getNumberOfLevels(count);
  struct cog *cog = mk_random_array(elements);
  rand_val(34534);
  cog = zipfianReads(cog, alpha, reads, elements);
  cog = zipfinize(cog, levels);
  printJITD(cog, 0);
  jsonJITD(cog, "test.json");
}

void testConvergence() 
{
  double alpha = 1;
  int size, range;
  int runs = 10;
  long totalSplays[runs], totalReads[runs], totalZipfinize[runs];
  long interval, levels, threshold, converge;
  struct cog *cog;

  size = 1000000;
  range = 1000;

  for (int i = 0; i < runs; i++) 
  {
    interval = 100;
    threshold = 50;
    converge = 1;

    totalSplays[i] = 0;
    totalReads[i] = 0;
    totalZipfinize[i] = 0;

    initInterval(interval, threshold);
    levels = getNumberOfLevels(range);
    sleep(1);
    rand_val(seedlessRandom());
    cog = getRandomArray(size, range);

    do 
    {
      interval = getInterval();
      totalReads[i] += interval;
      cog = zipfianReads(cog, alpha, interval, range);
      cog = zipfinize(cog, levels);
      totalSplays[i] += getSplays();
      totalZipfinize[i] += 1;
    } while (getSplays() > converge);
  }

  long splays = 0;
  long reads = 0;
  long zipfs = 0;
  for (int i = 0; i < runs; i++) 
  {
    splays += totalSplays[i];
    reads += totalReads[i];
    zipfs += totalZipfinize[i];
  }
  splays /= runs;
  reads /= runs;
  zipfs /= runs;
  printf("[AVERAGE] SPLAYS: %li READS: %li ZIPFINIZE: %li\n", splays, reads, zipfs);
}

void testSpeed() 
{
  struct cog *cog;
  struct timeval stop, start;
  int size, range, chunk, total;
  long interval, threshold, levels;
  double alpha = 1;
  size = 1000000;
  range = 1000;
  interval = 100;
  threshold = 50;

  rand_val(4357894);
  initInterval(interval, threshold);
  cog = getRandomArray(size, range);
  levels = getNumberOfLevels(range);

  total = 10000;

  gettimeofday(&start, NULL);

  while (1) 
  {
    if (total == 0) break;
    chunk = getInterval();
    if (chunk > total) chunk = total;

    cog = zipfianReads(cog, alpha, chunk, range);

    // Median
    //struct cog *median = getMedian(cog);
    //cog = splay(cog, median);

    // Harvest
    //struct cog *harvest = getHarvest();
    //cog = splay(cog, harvest);

    // Zipfian Policy
    cog = zipfinize(cog, levels);

    total -= chunk;
  }

  gettimeofday(&stop, NULL);

  long long startms = start.tv_sec * 1000LL + start.tv_usec / 1000;
  long long stopms = stop.tv_sec * 1000LL + stop.tv_usec / 1000;
  printf("Took %lld milliseconds\n", stopms - startms);
}
#endif

int main(int argc, char **argv) 
{
  int rand_start = 42; 
  //time(NULL)
  //srand(rand_start);
  //test1();
  //srand(rand_start);
  //test2();
  //srand(rand_start);
  //test3();
  //srand(rand_start);
  //test4();
  //srand(rand_start);
  //test5();
  //splayTest();
  //srand(rand_start);
  test6(100);
  //test6(1000);
  //test6(10000);
  
  //test7(100);
  //test7(1000);
  //test7(10000);
  
  //struct cog *cog;
  //cog = mk_random_array(1000000);
  //timeRun(randomReads, cog, 1000, 1000000);
  test8(100);
  //test8(1000);
  //test8(10000);
  //test9(100);
  //test9(1000);
  //test9(10000);
  
  //testZipfinize();
  //readsCounterTest();
  
  #ifdef __ADVANCED
   //readsCounterTest();
   //printf("%lf\n", harmonic(2,1));
   //printf("%lu\n", getZipfCountAtCDF(100000, 1, 0.50));
   //printf("%lu\n", getNumberOfLevels(236));
   //testZipfinize();
   //testConvergence();
   //testSpeed();
   //test8(100);
  #endif
  
  #ifdef __HARVEST
    struct cog *harvestTestCog;
    long reads   = 1000;
    long range   = 1000000;
    int  doSplay = TRUE;
    int  steps  = 10;
    harvestTestCog = mk_random_array(range);
    harvestTestCog = splayOnHarvest(harvestTestCog, reads, range, doSplay, steps);
    //test8(100);
  #endif
}
