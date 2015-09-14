#include <stdio.h>
#include <stdlib.h>
#include "cog.h"
#include "data.h"

void free_cog(cog *c) { free(c); }

void clean(cog *c) {
  switch(c->type) {
    case COG_CONCAT:
    case COG_BTREE:
      free_cog(c);
      break;
    case COG_ARRAY:
    case COG_SORTEDARRAY:
      cleanup(c);
      break;
  }
}

void cleanup(cog *c){
  switch(c->type){
    case COG_CONCAT:
      cleanup(c->data.concat.lhs);
      cleanup(c->data.concat.rhs);
      break;
    case COG_BTREE:
      cleanup(c->data.btree.lhs);
      cleanup(c->data.btree.rhs);
      break;
    case COG_ARRAY:
      buffer_release(c->data.array.records);
      break;
    case COG_SORTEDARRAY:
      buffer_release(c->data.sortedarray.records);
      break;
  }
  free_cog(c);
}

cog *make_concat( struct cog *lhs, struct cog *rhs ) {
  cog *ret = malloc(sizeof(struct cog));
  ret->type = COG_CONCAT;
  ret->data.concat.lhs = lhs;
  ret->data.concat.rhs = rhs;
  return ret;
}
cog *make_btree( struct cog *lhs, struct cog *rhs, long sep ) {
  cog *ret = malloc(sizeof(struct cog));
  ret->type = COG_BTREE;
  ret->data.btree.lhs = lhs;
  ret->data.btree.rhs = rhs;
  ret->data.btree.sep = sep;
  return ret;
}
cog *make_array( int start, int len, buffer records ) {
  cog *ret = malloc(sizeof(struct cog));
  ret->type = COG_ARRAY;
  ret->data.array.start = start;
  ret->data.array.len = len;
  ret->data.array.records = records;
  buffer_retain(records);
  return ret;
}
cog *make_sortedarray( int start, int len, buffer records ) {
  cog *ret = malloc(sizeof(struct cog));
  ret->type = COG_SORTEDARRAY;
  ret->data.sortedarray.start = start;
  ret->data.sortedarray.len = len;
  ret->data.sortedarray.records = records;
  buffer_retain(records);
  return ret;
}
extracted_components *make_extracted_components(struct cog *lhs, struct cog *rhs, long low_key, long high_key, iterator iter) {
  extracted_components *extracted_components = malloc(sizeof(struct extracted_components));
  extracted_components->lhs = lhs;
  extracted_components->rhs = rhs;
  extracted_components->low_key = low_key;
  extracted_components->high_key = high_key;
  extracted_components->iter = iter;
  return extracted_components;
}

void cleanup_extracted_components(extracted_components *extracted_components) {
  cleanup(extracted_components->lhs);
  cleanup(extracted_components->rhs);
  iter_cleanup(extracted_components->iter);
}

iterator scan( struct cog *cog, long low, long high ) {
  if( cog->type == COG_BTREE ) 
    {
      struct cog *a = cog->data.btree.lhs;
      struct cog *b = cog->data.btree.rhs;
      long sep = cog->data.btree.sep;
      if( sep <= low ) { return scan(b, low, high); }
    }
  if( cog->type == COG_BTREE ) 
    {
      struct cog *a = cog->data.btree.lhs;
      struct cog *b = cog->data.btree.rhs;
      long sep = cog->data.btree.sep;
      if( sep >= high ) { return scan(a, low, high); }
    }
  if( cog->type == COG_CONCAT || cog->type == COG_BTREE ) 
    {
        struct cog *a;
        struct cog *b;
      if(cog->type == COG_CONCAT) {
        a = cog->data.concat.lhs;
        b = cog->data.concat.rhs;
      } else {
        a = cog->data.btree.lhs;
        b = cog->data.btree.rhs;
      }
      return iter_concat(
      scan(a, low, high), 
      scan(b, low, high)
    );
    }
  if( cog->type == COG_SORTEDARRAY ) 
    {
      int start = cog->data.sortedarray.start;
      int len = cog->data.sortedarray.len;
      buffer b = cog->data.sortedarray.records;
      return array_binarysearch_scan(low, high, start, len, b);
    }
  if( cog->type == COG_ARRAY || cog->type == COG_SORTEDARRAY ) 
    {
      int start = cog->data.array.start;
      int len = cog->data.array.len;
      buffer b = cog->data.array.records;
      return array_scan(low, high, start, len, b);
    }

  fprintf(stderr, "Unmatched case in 'scan'\n");
  exit(-1);
}

//Scans the full the Cog and returns iterator.
iterator scan_full_array( struct cog *cog) {
  if( cog->type == COG_BTREE )
    {
      struct cog *a = cog->data.btree.lhs;
      struct cog *b = cog->data.btree.rhs;
      return iter_concat(
      scan_full_array(a),
      scan_full_array(b)
    );      
    }
  if( cog->type == COG_CONCAT )
    {
      struct cog *a = cog->data.concat.lhs;
      struct cog *b = cog->data.concat.rhs;
      return iter_concat(
      scan_full_array(a),
      scan_full_array(b)
    );
    }
  if( cog->type == COG_SORTEDARRAY )
    {
      int start = cog->data.array.start;
      int len = cog->data.array.len;
      struct record *in = buffer_start(cog->data.array.records, start);
      struct buffer *out = buffer_alloc(cog->data.sortedarray.len + 1);
      int i, tgt;
      iterator ret;
      for(i = 0, tgt = 0; i < len; i++){
        record_copy(&(in[i]), &(out->data[tgt]));
        tgt++;
      }
      ret = array_iter_alloc(out, 0, tgt);
      return ret;
    }
  if( cog->type == COG_ARRAY )
    {
      int start = cog->data.array.start;
      int len = cog->data.array.len;
      struct record *in = buffer_start(cog->data.array.records, start);
      struct buffer *out = buffer_alloc(cog->data.array.len );
      int i, tgt;
      iterator ret;
      for(i = 0, tgt = 0; i < len; i++){
        record_copy(&(in[i]), &(out->data[tgt]));
        tgt++;
      }
      ret = array_iter_alloc(out, 0, tgt);
      return ret;
    }
    fprintf(stderr, "Invalid Cog Type %d\n", cog->type);
    exit(-1);
}

// Calculates the length of the Cog.
int cog_length(struct cog *cog) {
  if(cog->type == COG_BTREE) {
    return cog_length(cog->data.btree.lhs) + cog_length(cog->data.btree.rhs);
  } else if(cog->type == COG_CONCAT) {
    return cog_length(cog->data.concat.lhs) + cog_length(cog->data.concat.rhs);
  } else if(cog->type == COG_ARRAY) {
    return cog->data.array.len;
  } else {
    return cog->data.sortedarray.len;
  }
}

list *create_list() {
  list *head = (struct list*)malloc(sizeof(list));
  return head;
}

void cleanup_list(list *list) {
  struct list *temp;
  int count=0;
  while(list != NULL) {
    temp = list->next;
    if(list->cog != NULL) {
      count++;
      cleanup(list->cog);
    }
    free(list);
    list = temp;
  }
  printf("Free Count %d\n", count);
}

int list_has_next(list *list) {
  if(list->next != NULL) {
    return 1;
  } else  {
    return 0;
  }
}

list *get_cog_from_list(list *list, struct cog *cog) {
  cog->type = list->cog->type;
  cog->data.sortedarray.start = list->cog->data.sortedarray.start;
  cog->data.sortedarray.len = list->cog->data.sortedarray.len;
  cog->data.sortedarray.records = list->cog->data.sortedarray.records;
  return list->next;
}

int get_length(list *list) {
  int count =0;
  while(list != NULL && list->cog != NULL) {
  count++;
  list = list->next;
  }
  return count;
}

void convert_to_sortedarray(struct cog *cog) {
  cog->type = COG_SORTEDARRAY;
  cog->data.sortedarray.records = cog->data.array.records;
  cog->data.sortedarray.start = 0;
  cog->data.sortedarray.len = cog->data.sortedarray.len;
}

struct cog *array_load(iterator iter,int len) {
  int i=0;
  record r = malloc(sizeof(struct record));
  buffer out = buffer_alloc(len);
  record buf = out->data;
  for(i = 0; i < len ; i++) {
    if(iter_has_next(iter)) {
      iter_next(iter, r);
      record_set(&(buf[i]), r->key, r->value);
    }
  }
  free(r);
  return make_array(0, len, out);
}

long cog_min(struct cog *c) {
  long min;
  switch(c->type) {
  case COG_CONCAT:
    min =  math_min(cog_min(c->data.concat.lhs), cog_min(c->data.concat.rhs));
    break;
  case COG_BTREE:
    min = cog_min(c->data.btree.lhs);
    if(min ==  MAX_VALUE) {
      min = cog_min(c->data.btree.rhs);
    }
    break;
  case COG_SORTEDARRAY:
    if(c->data.sortedarray.records == NULL) {
      min = MAX_VALUE;
    } else {
      min = buffer_key(c->data.sortedarray.records, c->data.sortedarray.start);
    }
    break;
  case COG_ARRAY:
    fprintf(stderr, "Unhandled case: Array min\n");
    exit(-1);
  }
  return min;
}

stack_triple *create_stack() {
  stack_triple *stack = malloc(sizeof(stack_triple));
  stack->triple = NULL;
  stack->next = NULL;
  return stack;
}

void push_stack(struct triple *t, stack_triple **top) {
  stack_triple *new = create_stack();
  new->triple = t;
  new->next = *top;
  *top = new;
}

stack_triple *pop_stack(stack_triple **top) {
  stack_triple *temp = *top;
  *top = (*top)->next;
  return temp;
}

int stack_empty(stack_triple **top) {
  if((*top)->triple == NULL) {
    return 1;
  } else {
    return 0;
  }
}

int peek_depth(stack_triple **top) {
  return (*top)->triple->depth;
}

triple *create_triple() {
  triple *new = malloc(sizeof(struct triple));
  new->cog = NULL;
  return new;
}

double_struct *create_double_struct() {
  double_struct *new = malloc(sizeof(double_struct));
  return new;
}

void cleanup_stack(stack_triple *stack) {
  free(stack->triple);
  free(stack);
}

void fold_append(stack_triple **stack, struct cog *c, long low) {
  int depth = 0;
  while(stack_empty(stack) !=1 && depth == peek_depth(stack)) {
    stack_triple *temp_stack = pop_stack(stack);
    c = make_btree(temp_stack->triple->cog, c, low);
    low = temp_stack->triple->key;
    depth++;
    cleanup_stack(temp_stack);
  }
  triple *entry = create_triple();
  entry->cog = c;
  entry->depth = depth;
  entry->key = low;
  push_stack(entry, stack);
}

cog *fold(stack_triple **stack) {
  if(stack_empty(stack)) return NULL;
  stack_triple *head = pop_stack(stack);
  while(stack_empty(stack) != 1) {
    stack_triple *prev = pop_stack(stack);
    head->triple->cog = make_btree(prev->triple->cog, head->triple->cog, head->triple->key);
    head->triple->key = prev->triple->key;
    cleanup_stack(prev);
  }
  cog *c = head->triple->cog;
  cleanup_stack(head);
  return c;
}

