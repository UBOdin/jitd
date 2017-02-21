#ifndef HEAVYHIT_H_SHEILD
#define HEAVYHIT_H_SHEILD

#include <stdlib.h>

typedef struct heavyhit {
  int key_shift;
  int lower_bound;
  int upper_bound;
  int hot_interval;
  int cold_interval;
  double hot_data_fraction;
  double hot_access_fraction;
} heavyhit;

struct heavyhit *create_heavyhit(
    int key_shift,
    int lower_bound, 
    int upper_bound, 
    double hot_data_fraction, 
    double hot_access_fraction
    );

int next_value(struct heavyhit *h);

double mean(struct heavyhit *h);

void free_heavyhit(heavyhit *h);

#endif //HEAVYHIT_H_SHEILD
