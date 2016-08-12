#include "cog.h"
#include "cracker.h"
#include "splay.h"
#include "util.h"
#include "zipf.h"
#include "dist_reads.h"

#define BUFFER_SIZE 10
#define KEY_RANGE 1000000

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
