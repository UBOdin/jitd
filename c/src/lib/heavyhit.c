#include "heavyhit.h"
#include "zipf.h"
#include <stdio.h>
#include <stdlib.h>

struct heavyhit *create_heavyhit(
    int key_shift,
    int lower_bound, 
    int upper_bound, 
    double hot_data_fraction, 
    double hot_access_fraction
    )
{
  if (hot_data_fraction < 0.0 || hot_data_fraction > 1.0){
    printf("Hot data fraction out of range. Setting to 0.0\n");
    hot_data_fraction = 0.0;
  }
  if (hot_access_fraction < 0.0 || hot_access_fraction > 1.0){
    printf("Hot access fraction out of range. Setting to 0.0\n");
    hot_access_fraction = 0.0;
  }
  if (lower_bound > upper_bound){
    printf("Upper bound of Hotspot generator smaller than the lower bound. ");
    printf("Swapping the values.\n");
    int temp = lower_bound;
    lower_bound = upper_bound;
    upper_bound = temp;
  }
  heavyhit *h = malloc(sizeof(struct heavyhit));
  h->key_shift = key_shift;
  h->lower_bound = lower_bound;
  h->upper_bound = upper_bound;
  h->hot_data_fraction = hot_data_fraction;
  h->hot_access_fraction = hot_access_fraction;
  int interval = upper_bound - lower_bound + 1;
  h->hot_interval = (int)(interval * hot_data_fraction);
  h->cold_interval = interval - h->hot_interval;
  return h;
}

int next_value(struct heavyhit *h)
{
  int value = 0;
  if (rand_val(0) < h->hot_access_fraction) {
    /* Choose a value from the hot set */
    value = h->lower_bound + (rand() % h->hot_interval);
  } else {
    /* Choose a value from the cold set */
    value = h->lower_bound + h-> hot_interval + (rand() % h->cold_interval);
  }
  return value;
}

double mean(struct heavyhit *h)
{
  return h->hot_access_fraction * (h->lower_bound + h->hot_interval/2.0)
    + (1 - h->hot_access_fraction) * 
    (h->lower_bound + h->hot_interval + h->cold_interval/2.0);
}

void free_heavyhit(heavyhit *h)
{
    free(h);
}
