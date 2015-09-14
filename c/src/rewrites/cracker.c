#include <stdio.h>
#include <stdlib.h>
#include "cog.h"
#include "data.h"

cog *pushdown_concats(cog *c, long low, long high) {
  if(c->type == COG_BTREE) {
    cog *lhs = c->data.btree.lhs;
    cog *rhs = c->data.btree.rhs;
  
    if(c->data.btree.sep <= high) {
      rhs = pushdown_concats(rhs, low, high);
    }  

    if(c->data.btree.sep >= low) {
      lhs = pushdown_concats(lhs, low, high);
    }

    if(lhs != c->data.btree.lhs || rhs != c->data.btree.rhs) {
      cog *ret = malloc(sizeof(struct cog));
      ret->type = COG_BTREE;
      ret->data.btree.lhs = lhs;
      ret->data.btree.rhs = rhs;
      cleanup(c);
      return ret;
    } else {
      return c;
    }
  } else if(c->type == COG_CONCAT) {
    cog *lhs = pushdown_concats(c->data.concat.lhs, low, high);
    cog *rhs = pushdown_concats(c->data.concat.rhs, low, high);
    if(rhs->type == COG_BTREE && lhs->type == COG_ARRAY) {
      cog *tmp = rhs;
      rhs = lhs;
      lhs = tmp;
    }
    if(lhs->type == COG_BTREE && (rhs->type == COG_ARRAY || rhs->type == COG_SORTEDARRAY)) {
      int start, count;
      cog *array_cog;
      if(rhs->type == COG_ARRAY) {
        array_cog = rhs;
        start = array_cog->data.array.start;
        count = rhs->data.array.len;
      } else {
        array_cog = c->data.concat.rhs;
        start = array_cog->data.sortedarray.start;
        count = rhs->data.sortedarray.len;
      }
      int radix_pos = radix(array_cog, start, count, lhs->data.btree.sep);
      cog *new_lhs = lhs->data.btree.lhs;
      cog *new_rhs = lhs->data.btree.rhs;
      
      if(radix_pos > 0) {
        cog *array1;
        buffer buf;
        if(array_cog->type == COG_ARRAY) {
          buf =  array_cog->data.array.records;
        } else {
          buf = array_cog->data.sortedarray.records;
        }
        array1 = make_array(start, radix_pos, buf);
        new_lhs = make_concat(lhs->data.btree.lhs, array1);
      }

      if(radix_pos < count){
        cog *array2;
        buffer buf;
        if(array_cog->type == COG_ARRAY) {
          buf = array_cog->data.array.records;
        } else {
          buf = array_cog->data.sortedarray.records;
        }
        array2 = make_array(start+radix_pos, count-radix_pos, buf);
        new_rhs = make_concat(lhs->data.btree.rhs, array2);
      }
      cog *btree;
      btree = make_btree(new_lhs, new_rhs, lhs->data.btree.sep);
      cleanup(c);
      return pushdown_concats(btree, low, high); 
    }
    if(lhs != c->data.concat.lhs || rhs != c->data.concat.rhs) {
      return make_concat(lhs, rhs);
    } else {
      return c;
    }
  } else {
    return c;
  }
}


cog *crack_one(cog *c, long val) {
  if(c->type == COG_SORTEDARRAY) {
    return c;
  } else if(c->type == COG_BTREE) {
    cog *lhs = c->data.btree.lhs;
    cog *rhs = c->data.btree.rhs;
    if(val == c->data.btree.sep) {
      return c;
    } else if(val < c->data.btree.sep){
      lhs = crack_one(lhs, val);
    } else {
      rhs = crack_one(rhs, val);
    }
    if(c->data.btree.lhs != lhs || c->data.btree.rhs != rhs) {
      long val = c->data.btree.sep;
      return make_btree(lhs, rhs, val);
    } else {
      return c;
    }
  } else if(c->type == COG_ARRAY) {
    int low, high;
    low = c->data.array.start;
    high = low + c->data.array.len;
    int radixPos = low;
    buffer buf = c->data.array.records;
    record r = buf->data;
    int i;
    for(i = low; i < high; i++) {
      if(r[i].key < val) {
        if(i > radixPos) {
          record_swap(&(r[i]), &(r[radixPos]));
        }
        radixPos++;
      }
    }
    cog *array1 = make_array(low, radixPos - low, c->data.array.records);
    cog *array2 = make_array(radixPos, high - radixPos, c->data.array.records);
    cleanup(c);
    return make_btree(array1, array2, val);
  } else {
    buffer out = buffer_alloc(cog_length(c) + 1);
    record buf = out->data;
    int lowIdx =0, highIdx = cog_length(c) - 1;
    iterator iter = scan_full_array(c);
    record r = malloc(sizeof(struct record));
    while(iter_has_next(iter)) {
      iter_next(iter, r);
      long key = r->key;
      if(key < val) {
        record_set(&(buf[lowIdx]), key, r->value);
        lowIdx++;
      } else {
        record_set(&(buf[highIdx]), key, r->value);
        highIdx--;
      } 
    }
    iter_cleanup(iter);
    cog *array1 = make_array(0, lowIdx, out);
    cog *array2 = make_array(highIdx + 1, out->size - highIdx - 1, out);
    cleanup(c);
    return make_btree(array1, array2, val);
  }
}

cog *crack_scan(cog *c, long low, long high) {
  if(c->type == COG_SORTEDARRAY) {
    return c;
  } else if(c->type == COG_BTREE) {
    cog *lhs = c->data.btree.lhs;
    cog *rhs = c->data.btree.rhs;
    if(low < c->data.btree.sep) {
      if(high < c->data.btree.sep) {
        lhs = crack_scan(lhs, low, high);
      } else {
        lhs = crack_one(lhs, low);
      }
    }
    if(high > c->data.btree.sep) {
      if(low > c->data.btree.sep) {
        rhs = crack_scan(rhs, low, high);
      } else {
        rhs = crack_one(rhs, high);
      }
    }
    if(c->data.btree.lhs != lhs || c->data.btree.rhs != rhs) {
      long sep = c->data.btree.sep;
      free(c);
      return make_btree(lhs, rhs, sep);
    } else {
      return c;
    }
  } else if(c->type == COG_ARRAY) {
    int lowIdx, highIdx;
    lowIdx = c->data.array.start;
    highIdx = lowIdx + c->data.array.len;
    buffer buf = c->data.array.records;

    int lowRadixPos = lowIdx, highRadixPos = highIdx;
    int i;
    record r = buf->data;
    for(i = lowIdx; i<highRadixPos; i++) {
      long key = buf->data[i].key;
      if(key < low) {
        if(i > lowRadixPos) {
            record_swap(&(r[i]), &(r[lowRadixPos]));
        }
        lowRadixPos++;
      } else if(key >= high) {
        record_swap(&(r[i]), &(r[highRadixPos-1]));
        highRadixPos--;
        i--;
      }
    }
    cog *array1 = make_array(lowIdx, lowRadixPos-lowIdx, c->data.array.records);
    cog *array2 = make_array(lowRadixPos, highRadixPos-lowRadixPos, c->data.array.records);
    cog *array3 = make_array(highRadixPos, highIdx-highRadixPos, c->data.array.records);
    cleanup(c);
    return make_btree(array1, make_btree(array2, array3, high), low);
  } else {
    buffer out = buffer_alloc(cog_length(c) + 1);
    record buf = out->data;
    int lowIdx = 0, midIdx = 0, highIdx = out->size - 1;
    iterator iter = scan_full_array(c);
    record r = malloc(sizeof(struct record));
    while(iter_has_next(iter)) {
      iter_next(iter, r);
      long key = r->key;
      if(key < low) {
        if(lowIdx < midIdx) {
          record_copy(&(buf[lowIdx]), &(buf[midIdx]));
        }
        record_set(&(buf[lowIdx]), key, r->value);
        lowIdx++;
        midIdx++;
      } else if(key < high) {
        record_set(&(buf[midIdx]), key, r->value);
        midIdx++;
      } else {
        record_set(&(buf[highIdx]), key, r->value);
        highIdx--;
      }
    }
    free(r);
    iter_cleanup(iter);
    cleanup(c);
    cog *array1 = make_array(0, lowIdx, out);
    cog *array2 = make_array(lowIdx, midIdx - lowIdx, out);
    cog *array3 = make_array(highIdx + 1, out->size - highIdx -1, out);
    return make_btree(array1, make_btree(array2, array3, high), low);   
  } 
}

cog *crack(cog *c, long low, long high) {
  cog *cog;
  cog = pushdown_concats(c, low, high);
  cog = crack_scan(cog, low, high);
  return cog;
}
