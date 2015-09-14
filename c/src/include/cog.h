#include "data.h"
#include <limits.h>
#define MAX_VALUE LONG_MIN 
#define MIN_VALUE LONG_MAX

#ifndef BTREE_H_SHIELD
#define BTREE_H_SHIELD

typedef enum {
  COG_CONCAT, COG_BTREE, COG_ARRAY, COG_SORTEDARRAY
} cog_type;

typedef struct cog {
  cog_type type;
  union {
    struct { struct cog *lhs; struct cog *rhs; } concat;
    struct { struct cog *lhs; struct cog *rhs; long sep; } btree;
    struct { int start; int len; buffer records; } array;
    struct { int start; int len; buffer records; } sortedarray;
  } data;
} cog;

typedef struct list {
  struct cog *cog;
  struct list *next;
} list;

typedef struct extracted_components {
  struct cog *lhs;
  struct cog *rhs;
  long low_key;
  long high_key;
  struct iterator *iter;
} extracted_components;

typedef struct triple {
  struct cog *cog;
  int depth;
  long key;
} triple;

typedef struct stack_triple {
  struct triple *triple;
  struct stack_triple *next;
} stack_triple;

typedef struct double_struct {
  struct cog *cog;
  iterator iter;
} double_struct;

void free_cog(cog *c);

cog *make_btree( struct cog *lhs, struct cog *rhs, long sep );

cog *make_array( int start, int len, buffer records );

cog *make_sortedarray( int start, int len, buffer records );

cog *make_concat( struct cog *lhs, struct cog *rhs );

extracted_components *make_extracted_components(struct cog *lhs, struct cog *rhs, long low_key, long high_key, iterator iter);

void cleanup_extracted_components(struct extracted_components *extracted_components);

iterator scan( struct cog *cog, long low, long high );

iterator scan_full_array(struct cog *cog); 

int cog_length(struct cog *cog);

void clean(cog *c);

void cleanup(cog *c);

list *create_list();

int list_has_next(list *list);

list *get_cog_from_list(list *list, struct cog *cog);

void cleanup_list(list *list);

void convert_to_sortedarray(struct cog *cog);

cog *array_load(iterator iter, int len);

long cog_min(struct cog *c);

stack_triple *create_stack();

double_struct *create_double_struct();

stack_triple *pop_stack(stack_triple **stack);

void cleanup_stack(stack_triple *stack);

void fold_append(stack_triple **stack, struct cog *c, long key);

cog *fold(stack_triple **stack);

int get_length(list *l);
#endif //BTREE_H_SHIELD
