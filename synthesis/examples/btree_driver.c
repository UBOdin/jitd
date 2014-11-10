#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "btree.h"
#include "cracker.h"
#include "adaptive_merge.h"

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
  test_scan(c, 100, 200);
  cleanup(c);
}

void test4() {
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

void test5() {
  printf("test 5\n");
  cog *c = make_concat(
            mk_random_array(BUFFER_SIZE),
            mk_random_array(BUFFER_SIZE)
          );
  iterator iter = malloc(sizeof(iterator));
  printf("Scan 1\n");
  c = amerge(c, 100, 200, iter);
  iter_dump(iter);
  printf("Scan 2\n");
  c = amerge(c, 300, 700, iter);
  iter_dump(iter);
  iter_cleanup(iter);
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
  srand(rand_start);
  test4();
  srand(rand_start);
  test5();
}
