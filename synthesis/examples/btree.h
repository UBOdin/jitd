#include "btree_lib.h"

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

void free_cog(cog *c);

cog *make_btree( struct cog *lhs, struct cog *rhs, long sep );

cog *make_array( int start, int len, buffer records );

cog *make_sortedarray( int start, int len, buffer records );

cog *make_concat( struct cog *lhs, struct cog *rhs );

iterator scan( struct cog *cog, long low, long high );

iterator scan_full_array(struct cog *cog); 

void cleanup(cog *c);

#endif //BTREE_H_SHIELD
