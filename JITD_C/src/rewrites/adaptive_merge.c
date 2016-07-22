#include <stdio.h>
#include <stdlib.h>
#include "cog.h"
#include "adaptive_merge.h"

cog *partition_cog(cog *c) {
  int records = cog_length(c);
  
  if(c->type == COG_ARRAY && records < BLOCK_SIZE) {
    record_sort(c->data.array.records->data, c->data.array.start, c->data.array.start + c->data.array.len);
    convert_to_sortedarray(c);
    return c;
  }
  
  iterator iter = scan_full_array(c);
  int count=0;

  cog *ret = NULL;
  while(records > 0) {
    count++;
    cog *store = array_load(iter, math_min(records, BLOCK_SIZE));
    records -= store->data.array.len;
    record_sort(store->data.array.records->data, 0, store->data.array.len);
    convert_to_sortedarray(store);
   
    if(ret == NULL) {
      ret = store;
    } else {
      ret = make_concat(ret, store);
    }
  }

  if(ret == NULL) {
    ret = make_sortedarray(0, 0, NULL);
  }
  cleanup(c);
  iter_cleanup(iter);
  return ret;
}

list *gather_partitions(list *list, cog *c) {
  if(c->type == COG_CONCAT) {
    list = gather_partitions(list, c->data.concat.lhs);
    list = gather_partitions(list, c->data.concat.rhs);
    free(c);
  } else {
    if(c->type == COG_ARRAY) {
      list = gather_partitions(list, partition_cog(c));
    } else {
      list->cog = c;      
      list->next = create_list();
      list = list->next;
      list->cog = NULL;
      list->next = NULL;
    }
  }
  return list; 
}

double_struct *amerge(cog *c, long low, long high) {
  double_struct *ret;
  if(c->type == COG_CONCAT) {
    return merge_partitions(c, low, high);
  } else if(c->type == COG_BTREE) {
    if(high <= c->data.btree.sep) {
      ret = amerge(c->data.btree.lhs, low, high);
      if(ret->cog != c->data.btree.lhs) {
        ret->cog = make_btree(ret->cog, c->data.btree.rhs, c->data.btree.sep);
      } else {
        ret->cog = c;
      }
    } else if(low >= c->data.btree.sep) {
      ret = amerge(c->data.btree.rhs, low, high);
      if(ret->cog != c->data.btree.rhs) {
        ret->cog = make_btree(c->data.btree.lhs, ret->cog, c->data.btree.sep);
      } else {
        ret->cog = c;
      }
    } else {
      ret = amerge(c->data.btree.lhs, low, c->data.btree.sep);
      double_struct *ret2 =  amerge(c->data.btree.rhs, c->data.btree.sep, high);
      if(ret->cog != c->data.btree.lhs || ret2->cog != c->data.btree.rhs) {
        ret->cog = make_btree(ret->cog, ret2->cog, c->data.btree.sep);
      } else {
        ret->cog = c;
      }
      ret->iter = iter_concat(ret->iter, ret2->iter);
      free(ret2);
    }
   
    if(c->type == COG_BTREE) {
      if(c != ret->cog)
        free(c);
    } 
    return ret;
  } else if(c->type == COG_SORTEDARRAY) {
    ret = create_double_struct();
    ret->iter = scan(c, low, high);
    ret->cog = c;
    return ret;
  } else {
    return amerge(partition_cog(c), low, high);
  }
}

double_struct *merge_partitions(cog *c, long low, long high) {
  list *list = create_list();
  int has_merge_work;
  gather_partitions(list, c);
  struct cog *right_replacement = NULL;
  struct cog *left_replacement = NULL;
  struct list *list_iter;
  struct iter_list *tail; 
  struct stack_triple *stack = create_stack();
  struct iter_list *head_list = malloc(sizeof(struct iter_list));
  tail = head_list;
  list_iter = list;
  iterator merge_iter;
  cog *c1 = malloc(sizeof(struct cog));
  while(list_has_next(list_iter)) {
    list_iter = get_cog_from_list(list_iter, c1);
    struct extracted_components *components = extract_partitions(c1, low, high);
    if(components->iter != NULL) {
      has_merge_work = 1;
      tail = iter_list_add(tail, components->iter);
    }

    if(components->rhs != NULL) {
      if(right_replacement == NULL) {
        right_replacement = components->rhs;
      } else {
        right_replacement = make_concat(right_replacement, components->rhs); 
      }
    }

    if(components->lhs != NULL) {
      if(left_replacement == NULL) {
        left_replacement = components->lhs;
      } else {
        left_replacement = make_concat(left_replacement, components->lhs);
      }
    }
    free(components);
  }
  cleanup_list(list);
  free(c1);
  if(has_merge_work != 0) {
    double_struct *ret = create_double_struct();
    merge_iter = iter_merge(head_list);
    record r = malloc(sizeof(struct record));
    while(1) {
      int i;
      buffer out = buffer_alloc(MERGE_BLOCK_SIZE);
      record buf = out->data;
      for(i = 0; i < MERGE_BLOCK_SIZE && iter_has_next(merge_iter); i++) {
        iter_next(merge_iter, r);
        record_set(&buf[i], r->key, r->value);
      }
      if(i == 0) {
        break;
      }
      struct cog *buffer = make_sortedarray(0, i, out);
      fold_append(&stack, buffer, cog_min(buffer));
      if(i < MERGE_BLOCK_SIZE) {
        break;
      }
    }
    cog *root = fold(&stack);
    if(root == NULL) {
      root = make_btree(left_replacement, right_replacement, high);
    } else {
      ret->iter = scan(root, low, high);
      if(left_replacement != NULL && cog_length(left_replacement) != 0) {
        root = make_btree(left_replacement, root, cog_min(root));
      }
      if(right_replacement != NULL && cog_length(right_replacement) != 0) {
        root = make_btree(root, right_replacement, cog_min(right_replacement));
      }
    }
    free(r);
    cleanup_stack(stack);
    iter_list_cleanup(head_list);
    iter_cleanup(merge_iter);
    ret->cog = root;
    return ret;
  } else {
    double_struct *ret;
    ret = amerge(list->cog, low, high);
    if(left_replacement != NULL) {
      ret->cog = make_concat(ret->cog, left_replacement);
    }
    if(right_replacement != NULL) {
      ret->cog = make_concat(ret->cog, right_replacement);
    }
    return ret;
  }
}

extracted_components *extract_partitions(cog *c, long low, long high) {
  double_struct *ret_struct;
  ret_struct = amerge(c, low, high);
  c = ret_struct->cog;
  iter_cleanup(ret_struct->iter);
  free(ret_struct);
  extracted_components *ret; 
  if(c->type == COG_BTREE) {
    if(c->data.btree.sep <= low) {
      ret = extract_partitions(c->data.btree.rhs, low, high);
      ret->lhs = ret->lhs == NULL ? c->data.btree.lhs : make_btree(c->data.btree.lhs, ret->lhs, c->data.btree.sep);
    } else if(c->data.btree.sep >= high) {
      ret = extract_partitions(c->data.btree.lhs, low, high);
      ret->rhs = ret->rhs == NULL ? c->data.btree.rhs : make_btree(ret->rhs, c->data.btree.rhs, c->data.btree.sep);
    } else {
      ret = extract_partitions(c->data.btree.lhs, low, c->data.btree.sep);
      struct extracted_components *ret2 = extract_partitions(c->data.btree.rhs, c->data.btree.sep, high);
      ret->rhs = ret2->rhs;

      if(ret->iter == NULL) {
        ret->iter = ret2->iter;
        ret->low_key = ret2->low_key;
        ret->high_key = ret2->high_key;
      } else if(ret2->iter != NULL) {
        ret->iter = iter_concat(ret->iter, ret2->iter);
        ret->low_key = math_min(ret->low_key, ret2->low_key);
        ret->high_key = math_max(ret->high_key, ret2->high_key);
      }
    }
    
    return ret;
  } else if(c->type == COG_SORTEDARRAY) {
    int start = c->data.sortedarray.start;
    int len = c->data.sortedarray.len;
    buffer b = c->data.sortedarray.records;
    int low_index = record_binarysearch(b->data, low, start, len);
    int high_index = record_binarysearch(b->data, high, start, len);
    while((low_index < high_index) && (b->data[low_index].key < low)) {
      low_index++;
    }
    while((low_index-1 >= start) && (b->data[low_index-1].key >= low)) {
      low_index--;
    }
    while((high_index < (start+len)) && (b->data[high_index].key <= high)) {
      high_index++;
    }
    cog *lhs = low_index > start ? make_sortedarray(start, low_index - start, b) : NULL;
    cog *rhs = high_index < (start+len) ? make_sortedarray(high_index, start + len - high_index, b) : NULL;
    iterator iter = low_index < high_index ? scan(c, low, high) : NULL;
    long low_key = iter == NULL ? MAX_VALUE :  buffer_key(b, low_index);
    long high_key = iter == NULL ? MIN_VALUE : buffer_key(b, high_index - 1);

    return make_extracted_components(lhs, rhs, low_key, high_key, iter);
  } else {
    return NULL;    
  }
}
