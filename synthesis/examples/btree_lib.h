
#ifndef BTREE_LIB_H_SHIELD
#define BTREE_LIB_H_SHIELD

// RECORDS
typedef struct record {
  long key;
  long value;
} *record;

// BUFFERS
typedef struct buffer {
  int refcount;
  int size;
  struct record data[0];
} *buffer;

// ITERATORS
typedef struct iterator_impl {
  int (*has_next)(void *data);
  void (*next)(void *data, record rec);
  void (*cleanup)(void *data);
} *iterator_impl;

typedef struct iterator {
  iterator_impl impl;
  void *data;
} *iterator;

// ITERATOR METHODS
int iter_has_next(iterator iter);
void iter_next(iterator iter, record r);
void iter_cleanup(iterator iter);
void iter_dump(iterator iter);
iterator iter_concat(iterator a, iterator b);

// BUFFER METHODS 
buffer buffer_alloc(int size);
void buffer_retain(buffer b);
void buffer_release(buffer b);
record buffer_start(buffer b, int idx);
long buffer_key(buffer b, int i);
long buffer_value(buffer b, int i);

// RECORD ARRAY OPERATIONS
void record_copy(record src, record dst);
void record_swap(record ir, record jr);
void record_sort(record r, int low, int high);
void record_dump(record r, int low, int high);
int record_binarysearch(record r, long key, int start, int len);

// ARRAY METHODS
iterator array_binarysearch_scan(long low, long high, int start, int len, buffer buffer);
iterator array_scan(long low, long high, int start, int len, buffer buffer);



#endif //BTREE_LIB_H_SHIELD