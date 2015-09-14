
#include <stdio.h>
#include <stdlib.h>
#include "data.h"

// BUFFERS
buffer buffer_alloc(int size)
{
  buffer b = malloc(sizeof(struct buffer) + sizeof(struct record) * size);
  b->refcount = 0;
  b->size = size;
  return b;
}

void buffer_retain(buffer b)
{
  b->refcount ++;
}

void buffer_release(buffer b)
{
  if(b != NULL) {
    b->refcount--;
    if(b->refcount < 1){
      free(b);
    }
  }
}

record buffer_start(buffer b, int idx){
  return &(b->data[idx]);
}

long buffer_key(buffer b, int i){
  return b->data[i].key;
}
long buffer_value(buffer b, int i){
  return b->data[i].value;
}

// RECORD ARRAY OPERATIONS

void record_copy(record src, record dst){
  dst->key   = src->key;
  dst->value = src->value;
}

void record_set(record src, long key, long value){
  src->key   = key;
  src->value = value;
}

void record_swap(record ir, record jr){
  struct record tmp;
  record_copy(ir, &tmp);
  record_copy(jr, ir);
  record_copy(&tmp, jr);
}
void record_dump(record r, int start, int end)
{
  int i;
  for(i = start; i < end; i++){
    printf(" %ld ", r[i].key);
  }
  printf("\n");
}
void record_sort(record r, int start, int end)
{
  int mid, i;
  long pivot;
//  printf("Sorting %d-%d\n", start, end);
//  record_dump(r, start, end);
  if(end - start <= 1){ return; }
  if(end - start == 2){
    if(r[start].key > r[start+1].key){
      record_swap(&(r[start]), &(r[start+1]));
    }
    return;
  }
  mid = start;
  i = start;
  pivot = r[start + rand() % (end-start)].key+1;
  while(i < end){
//    printf("         %ld \n", r[i].key);
    if(r[i].key < pivot){
      if(i != mid){
//        printf("          swap %d <-> %d\n", i, mid);
        record_swap(&(r[mid]), &(r[i]));
      }
      mid++;
    }
    i++;
  }
//  printf("  pivot: %ld @ %d in %d-%d\n", pivot, mid, start, end);
  if(mid > start){
//    printf("recur left\n");
    record_sort(r, start, mid);
  }
  if(mid < end){
//    printf("recur right\n");
    record_sort(r, mid, end);
  }
}

int record_binarysearch(record r, long key, int start, int len)
{
  int mid;
  if(len < 1) { return start; }
  if(len == 1) {
    if(key >= r[start].key){ return start; }
    else { return start+1; }
  }
  mid = start + (len/2);
  if(key == r[mid].key){
    return mid;
  }
  if(key < r[mid].key){
    return record_binarysearch(r, key, start, mid-start);
  } else {
    return record_binarysearch(r, key, mid, len + start - mid);
  }
}

int radix(buffer b, int low, int cnt, int radix){
  int radixPos =0;
  int i=0;
  for(i=0; i < cnt; i++) {
    if(b->data[i+low].key < radix) {
      if(radixPos < i) {
        record_swap(&(b->data[i+low]), &(b->data[radixPos+low]));
        radixPos++;
      }
    }
  }
  return radixPos;
}

// ITERATORS

int iter_has_next(iterator iter)
{
  return iter->impl->has_next(iter->data);
}
void iter_get_next(iterator iter, record r)
{
  return iter->impl->get_next(iter->data, r);
}
void iter_next(iterator iter, record r)
{
  iter->impl->next(iter->data, r);
}
void iter_cleanup(iterator iter)
{
  iter->impl->cleanup(iter->data);
  free(iter);
}
void iter_dump(iterator iter)
{
  struct record r;
  while(iter_has_next(iter)){
    iter_next(iter, &r);
    printf(" %5ld : %ld\n", r.key, r.value);
  }
}

iter_list *iter_list_add(iter_list *list, iterator iter) {
  list->iter = iter;
  list->next = (struct iter_list*)malloc(sizeof(iter_list));
  list = list->next;
  list->iter = NULL;
  list->next = NULL;
  return list;
}

void iter_list_cleanup(iter_list *list) {
  iter_list *temp;
  while(list != NULL) {
    temp = list->next;
    free(list);
    list = temp;
  }
}

int iter_list_length(iter_list *list) {
  int count=0;
  while(list != NULL && list->iter != NULL) {
    count++; 
    list = list->next;
  }
  return count;
}

typedef struct {
  int curr;
  int size;
  iterator iters[0];  
} merge_iter_data;


int merge_iter_has_next(void *vdata) {
  merge_iter_data *data = vdata;
  if(data->curr >= 0) { 
    if(iter_has_next(data->iters[data->curr]) == 0) {
          iter_cleanup(data->iters[data->curr]); 
          data->iters[data->curr] = NULL; 
    }
  }
  data->curr = -1;
  int i;
  for(i = 0; i < data->size && data->iters[i] == NULL; i++){}
  if(i >= data->size) { 
    return 0; 
  }
  record r = malloc(sizeof(struct record));
  iter_get_next(data->iters[i], r);
  long curr_lowest = r->key;
  data->curr = i;
  for(i = i + 1; i < data->size; i++){
    if(data->iters[i] != NULL){
      iter_get_next(data->iters[i], r);
      if(r->key < curr_lowest){
        curr_lowest = r->key;
        data->curr = i;
      }
    }
  }
  free(r);
  return 1;
}

void merge_iter_get_next(void *vdata, record r) {
  merge_iter_data *data = vdata;
  iter_get_next(data->iters[data->curr], r);
}

void merge_iter_next(void *vdata, record r) {
  merge_iter_data *data = vdata;
  iter_next(data->iters[data->curr], r);
}

void merge_iter_cleanup(void *vdata) {
  merge_iter_data *data = vdata;
  free(data); 
}

struct iterator_impl merge_iter_impl = {
  merge_iter_has_next,
  merge_iter_get_next,
  merge_iter_next,
  merge_iter_cleanup
};

iterator iter_merge(iter_list *list) {
  int size = iter_list_length(list);
  merge_iter_data *data = malloc(sizeof(merge_iter_data) +
                                  sizeof(struct iterator) * size);
  iterator iter = malloc(sizeof(struct iterator));
  iter->data = data;
  int i;
  for(i = 0; i < size; i++) {
    data->iters[i] = list->iter;
    list = list->next;
  }
  data->curr = 0;
  data->size = size;
  iter->impl = &merge_iter_impl;
  return iter;
}

// ARRAY ITERATOR OPERATIONS

typedef struct {
  buffer b;
  int curr;
  int end;
} array_iter_data;

int array_iter_has_next(void *vdata)
{
  array_iter_data *data = vdata;
  return data->curr < data->end;
}
void array_iter_next(void *vdata, record r)
{
  array_iter_data *data = vdata;
  record_copy(&(data->b->data[data->curr]), r);
  data->curr++;
}
void array_iter_get_next(void *vdata, record r)
{
  array_iter_data *data = vdata;
  record_copy(&(data->b->data[data->curr]), r);
}
void array_iter_cleanup(void *vdata)
{
  array_iter_data *data = vdata;
  buffer_release(data->b);
  free(data);
}

struct iterator_impl array_iter_impl = {
  array_iter_has_next,
  array_iter_get_next,
  array_iter_next,
  array_iter_cleanup
};

iterator array_iter_alloc(buffer b, int start, int end)
{
  array_iter_data *data = malloc(sizeof(array_iter_data));
  iterator iter = malloc(sizeof(struct iterator));
  iter->data = data;
  if(b != NULL) {
    buffer_retain(b);
  }
  data->b = b;
  data->curr = start;
  data->end = end;
  iter->impl = &array_iter_impl;
  return iter;
}

// CONCAT ITERATOR OPERATIONS

typedef struct {
  int idx;
  int cnt;
  iterator iters[0];
} concat_iter_data;

int concat_iter_has_next(void *vdata)
{
  concat_iter_data *data = vdata;
  return (data->idx < data->cnt);
}
void concat_iter_next(void *vdata, record r)
{
  concat_iter_data *data = vdata;
  if(data->idx < data->cnt){
    iter_next(data->iters[data->idx], r);
  }
  while( (data->idx < data->cnt) &&
         (!iter_has_next(data->iters[data->idx])))
  { 
    data->idx++;
  }
}
void concat_iter_get_next(void *vdata, record r)
{
  concat_iter_data *data = vdata;
  if(data->idx < data->cnt){
    iter_get_next(data->iters[data->idx], r);
  }
}
void concat_iter_cleanup(void *vdata)
{
  concat_iter_data *data = vdata;
  int i;
  for(i = 0; i < data->cnt; i++){
    iter_cleanup(data->iters[i]);
  }
  free(data);
}

struct iterator_impl concat_iter_impl = {
  concat_iter_has_next,
  concat_iter_get_next,
  concat_iter_next,
  concat_iter_cleanup
};

iterator iter_concat(iterator a, iterator b)
{
  concat_iter_data *data = malloc(sizeof(concat_iter_data) + 
                                  sizeof(struct iterator) * 2);
  iterator iter = malloc(sizeof(struct iterator));
  iter->data = data;
  data->iters[0] = a;
  data->iters[1] = b;
  data->idx = 0;
  data->cnt = 2;
  iter->impl = &concat_iter_impl;
  return iter;
}

// ARRAY OPERATIONS

iterator array_binarysearch_scan(long low, long high, int start, int len, buffer buffer)
{
  int start_idx = record_binarysearch(buffer->data, low, start, len);
  int end_idx = record_binarysearch(buffer->data, high, start, len);

  while((start_idx < end_idx) && (buffer->data[start_idx].key < low)) {
    start_idx ++;
  }
  while((start_idx-1 >= start) && (buffer->data[start_idx-1].key >= low)) {
    start_idx --;
  }
  while((end_idx < (start+len)) && (buffer->data[end_idx].key <= high)) {
    end_idx ++;
  }
  iterator ret = array_iter_alloc(buffer, start_idx, end_idx);
  return ret;
}
iterator array_scan(long low, long high, int start, int len, buffer buffer)
{
  struct record *in = buffer_start(buffer, start);
  struct buffer *out = buffer_alloc(len);
  int i, tgt;
  iterator ret;
  for(i = 0, tgt = 0; i < len; i++){
    long key = in[i].key;
    if((key >= low) && (key < high)){
      record_copy(&(in[i]), &(out->data[tgt]));
      tgt++;
    }
  }
  ret = array_iter_alloc(out, 0, tgt);
  return ret;
}

int math_min(int arg1, int arg2) {
  return arg1 > arg2 ? arg2 : arg1;
}

int math_max(int arg1, int arg2) {
  return arg1 > arg2 ? arg1 : arg2;
}
