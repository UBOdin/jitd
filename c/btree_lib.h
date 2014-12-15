
#ifndef BTREE_LIB_H_SHIELD
#define BTREE_LIB_H_SHIELD

// The functions and structures in this file follow a few conventions that
// may be nonstandard.  Please consult this list before defining new functions
// or changing anything
//
//  - Function and structure names use under_scores and not camelCase.
//  - Structures are defined in typedefs.  By convention `struct foo` represents
//    the actual structure, while `foo` represents a *pointer to the structure*
//      THIS MEANS THAT YOU MUST USE struct foo WHEN ALLOCATING MEMORY
//      this also means that structures are effectively PASS BY REFERENCE



//////////////////////// RECORDS
// A key/value pair.  Hypothetically we could replace this with a key-pointer 
// pair or similar datastructure somewhere down the line.
typedef struct record {
  long key;                 // The key of the record.
  long value;               // The value of the record (mostly ignored by 
                            // JITDs).
} *record;

//////////////////////// BUFFERS
// A header for a reference-counted array of records.  Due to the way C handles
// structure memory allocation, simply prepending this structure to the memory
// allocated for the array allows the data[] field to represent the buffer.  In
// other words, you can allocate a buffer as
//   malloc(sizeof(struct buffer) + (NUM_RECORDS * sizeof(struct record)));
// and free the entire thing with one operation.
// Memory management for buffers is abstracted for your convenience in the 
// buffer_* methods below.
typedef struct buffer {
  int refcount;             // The number of references to this buffer.
  int size;                 // The number of records in this buffer.
  struct record data[0];    // The data values in this buffer.  
} *buffer;

//////////////////////// ITERATORS
// A C "interface" definition for iterator objects.  Instances of iterator_impl
// provide method definitions for the primary operations on the iterator.
// Instances of struct iterator are actual object instances.  Use the iterator_*
// methods to access instances.
typedef struct iterator_impl {
  int (*has_next)(void *data);
  void (*get_next)(void *data, record rec);
  void (*next)(void *data, record rec);
  void (*cleanup)(void *data);
} *iterator_impl;

typedef struct iterator {
  iterator_impl impl;
  void *data;
} *iterator;
 
typedef struct iter_list {
  iterator iter;
  struct iter_list *next;
} iter_list;

//////////////////////// ITERATOR METHODS
// These methods operate over the iterator "interface" defined above.
// You should use these methods rather than calling the class-specific versions.
// That said, each iterator class has a class-specific constructor.  See 
// `array_scan` below for an example.

// iter_has_next(iter)
//     Check to see if `iter` has another record to read.  
//   iterator iter: The iterator to check
//   returns int: 0 if the iterator is complete, nonzero otherwise
int iter_has_next(iterator iter);

// iter_next(iter, r)
//      Read the next record from the iterator.  Results are undefined if 
//      iter_has_next indicates that there are no more records to read.
//   iterator iter: The iterator to read from
//   record r: A pointer to a buffer to read the record into
void iter_next(iterator iter, record r);

void iter_get_next(iterator iter, record r);

// iter_cleanup(iter)
//      Release an iterator.  Analogous to `free(iter)`, but also runs 
//      iterator-specific cleanup code.
//   iterator iter: The iterator to free
void iter_cleanup(iterator iter);

// iter_dump(iter)
//      Print all of the records returned by `iter` to stdout
//    iterator iter: The iterator to dump to stdout
void iter_dump(iterator iter);

// iter_concat(a, b)
//      Create an iterator that returns all of the records in `a` and then
//      returns all of the records in `b`
//   iterator a: The iterator to read from first
//   iterator b: The iterator to read from second
//   returns iterator: An iterator that iterates over the records in `a` and
//                     then the records in `b`
iterator iter_concat(iterator a, iterator b);


iterator iter_merge(iter_list *list);
////////////////////////  BUFFER METHODS 
// These methods operate over the buffer interface defined above.  You should 
// use `buffer_alloc`, `buffer_retain`, and `buffer_release` instead of native 
// memory management (i.e., malloc/free)

// buffer_alloc(size)
//     Allocate a buffer containing `size` records.  The reference count of the
//     buffer will be initialized to 1.
//   int size: The number of records in the allocated buffer.
//   returns buffer: A pointer to the allocated buffer.
buffer buffer_alloc(int size);

// buffer_retain(b)
//     Increment the buffer's reference count
//   buffer b: The buffer to retain
void buffer_retain(buffer b);

// buffer_release(b)
//     Decrement the buffer's reference count and free it if the count reaches
//     zero. Analogous to `free`, but reference-counted.
//   buffer b: The buffer to release
void buffer_release(buffer b);

// buffer_start(b, idx)
//     Utility method for computing the pointer to the specified index.
//   buffer b: The buffer to reference into
//   int idx: The index of the record to return a pointer to
//   returns record: A pointer to record `idx` of buffer `b`
record buffer_start(buffer b, int idx);

// buffer_key(b, i)
//     Utility method for looking up the key field of the record at a specified 
//     index.
//   buffer b: The buffer to reference into
//   int idx: The index of the record to return a pointer to
//   returns long: The key of record `idx` of buffer `b`
long buffer_key(buffer b, int idx);

// buffer_value(b, i)
//     Utility method for looking up the value field of the record at a 
//     specified index.
//   buffer b: The buffer to reference into
//   int idx: The index of the record to return a pointer to
//   returns long: The key of record `idx` of buffer `b`
long buffer_value(buffer b, int idx);

//////////////////////// RECORD ARRAY OPERATIONS
// Utility methods for manipulating arbitrary record arrays and pointers.  These
// methods may be used with `buffer`s, or any other types of record 
// array/pointer

// record_copy(src, dst)
//     Utility method for copying records.  The memory pointed to by `dst` is
//     overwritten by the contents pointed to by `src`
//   record src: Pointer to the record to copy.
//   record dst: Pointer to the space to copy into.
void record_copy(record src, record dst);

// record_swap(ir, jr)
//     Utility method for swapping records.  After the call, the contents of
//     the records pointed to by `ir` and `jr` will be swapped.
//   record ir: Pointer to a record to be swapped with `jr`
//   record jr: Pointer to a record to be swapped with `ir`
void record_swap(record ir, record jr);

// record_sort(r, low, high)
//     Sort records from (inclusive) `r[start]` to (exclusive) `r[end]`.  
//     Implemented as quicksort
//   record r: The array of records to sort part of
//   int start: The index to start sorting from
//   int end: The index following the last record to sort.
void record_sort(record r, int start, int end);

// record_dump(r, start, end)
//     Print records from (inclusive) `r[start]` to (exclusive) `r[end]`.  
//   record r: The array of records to print part of
//   int start: The first record to print
//   int end: The index following the last record to print
void record_dump(record r, int start, int end);

// record_binarysearch(r, key, start, len)
//     Search for `key` in records from (inclusive) `r[start]` to (exclusive)
//     `r[start+len]`.  `r` must be sorted by the record keys.  If `r` contains 
//     at least one instance of `key`, the index of one such record will be 
//     returned.  If `r` does not contain `key` the highest index of a record
//     with a key lower than `key` is returned instead.
//  record r: The array of records to search
//  long key: The key to search for
//  int start: The first index to search
//  int len: The number of records following start to search
int record_binarysearch(record r, long key, int start, int len);

void record_set(record src, long key, long value);

//////////////////////// ARRAY ITERATOR CONSTRUCTORS
// Constructors for array iterators.  Use binarysearch_scan if the buffer is
// sorted.  Use scan if the buffer is not sorted.

// array_binarysearch_scan(low, high, start, len, buffer)
//     Create an iterator over records in `buffer` with keys in the range 
//     `[low, high)` and indexes in the range `[start, start+len)`.  `buffer` 
//     must be sorted: Complexity will be O(|records returned|)
//  long low: The inclusive lower bound on keys for the iterator to return.
//  long high: The exclusive upper bound on keys for the iterator to return.
//  int start: The inclusive first index for the iterator to scan.
//  int len: The number of keys following start for the iterator to scan.
//  buffer buffer: A sorted buffer to scan.
iterator array_binarysearch_scan(long low, long high, int start, int len, buffer buffer);

// array_scan(low, high, start, len, buffer)
//     Create an iterator over records in `buffer` with keys in the range 
//     `[low, high)` and indexes in the range `[start, start+len)`.  `buffer` 
//     does not need to be sorted: Complexity will be O(|buffer|)
//  long low: The inclusive lower bound on keys for the iterator to return.
//  long high: The exclusive upper bound on keys for the iterator to return.
//  int start: The inclusive first index for the iterator to scan.
//  int len: The number of keys following start for the iterator to scan.
//  buffer buffer: A sorted buffer to scan.
iterator array_scan(long low, long high, int start, int len, buffer buffer);

iterator array_iter_alloc(buffer b, int start, int end);

iter_list *iter_list_add(iter_list *list, iterator iter);

void iter_list_cleanup(iter_list *list);

int iter_list_length(iter_list *list); 

int math_min(int arg1, int arg2);

int math_max(int arg1, int arg2);
#endif //BTREE_LIB_H_SHIELD
