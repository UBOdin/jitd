#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "btree.c"

#define BUFFER_SIZE 10
#define KEY_RANGE   1000

buffer mk_random_buffer(int size)
{
  buffer b = buffer_alloc(size);
  int i;
  for(i = 0; i < size; i++){
    b->data[i].key = rand() % KEY_RANGE;
    b->data[i].value = rand();
  }
  record_dump(b->data, 0, size);
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

void cleanup(cog *c){
  switch(c->type){
    case COG_CONCAT:
    case COG_BTREE:
      cleanup(c->data.concat.lhs);
      cleanup(c->data.concat.rhs);
      break;
    case COG_ARRAY:
    case COG_SORTEDARRAY:
      buffer_release(c->data.array.records);
      break;
  }
  free_cog(c);
}

void test_scan(cog *c, long low, long high)
{
  iterator iter = scan(c, low, high);
  iter_dump(iter);
  iter_cleanup(iter);
}

void test1()
{
  printf("test 1\n");
  cog *c = mk_random_array(BUFFER_SIZE);
  test_scan(c, 200, 700);
  cleanup(c);
}

void test2()
{
  printf("test 2\n");
  cog *c = mk_sorted_array(BUFFER_SIZE);
  test_scan(c, 200, 700);
  cleanup(c);
}
void test3()
{
  printf("test 3\n");
  cog *c = 
    make_concat(
      mk_random_array(BUFFER_SIZE),
      mk_random_array(BUFFER_SIZE)
    );
  test_scan(c, 200, 700);
  cleanup(c);
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
}