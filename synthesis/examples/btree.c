#include <stdio.h>
#include <stdlib.h>
#include "btree.h"
#include "btree_lib.h"

void free_cog(cog *c) { free(c); }

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
      buffer_release(out);
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
      buffer_release(out);
      return ret;
    }
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
