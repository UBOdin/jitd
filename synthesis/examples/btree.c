#include "jitd_lib.c"
#include "btree_lib.h"

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

void free_cog(cog *c) { free(c); }
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
  return ret;
}
cog *make_sortedarray( int start, int len, buffer records ) {
  cog *ret = malloc(sizeof(struct cog));
  ret->type = COG_SORTEDARRAY;
  ret->data.sortedarray.start = start;
  ret->data.sortedarray.len = len;
  ret->data.sortedarray.records = records;
  return ret;
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
      struct cog *a = cog->data.concat.lhs;
      struct cog *b = cog->data.concat.rhs;
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
