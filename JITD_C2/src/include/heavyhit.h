#ifndef HEAVYHIT_H_SHEILD
#define HEAVYHIT_H_SHEILD

#include <stdlib.h>

typedef struct heavyhit
{
  int lower_bound;
  int upper_bound;
  int hot_interval;
  int cold_interval;
  double hot_data_fraction;
  double hot_access_fraction;
}

struct heavyhit *create_heavyhit(
    int lower_bound, 
    int upper_bound, 
    double hot_data_fraction, 
    double hot_access_fraction
    );

int next_value(struct *heavyhit);

double mean(struct *heavyhit);

#endif //HEAVYHIT_H_SHEILD
