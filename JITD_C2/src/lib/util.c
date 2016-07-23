#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "cog.h"
#include "cracker.h"
#include "splay.h"
#include "zipf.h"


/**
 * Prints a detailed representation of a single Array or Sorted Array cog.
 *
 * @param cog - cog to print
 */
void printArrayCog(struct cog *cog) {
#ifndef __ADVANCED
  printf("[");

  if (cog->type == COG_ARRAY) {
    for (int i = 0; i < cog->data.array.len; i++) {
      int offset = cog->data.array.start;
      printf("%ld", cog->data.array.records->data[i + offset].key);
      if (i + 1 < cog->data.array.len) printf(",");
    }

    printf("]");
  }

  if (cog->type == COG_SORTEDARRAY) {
    for (int i = 0; i < cog->data.sortedarray.len; i++) {
      int offset = cog->data.sortedarray.start;
      printf("%ld", cog->data.sortedarray.records->data[i + offset].key);
      if (i + 1 < cog->data.sortedarray.len) printf(",");
    }

    printf(">");
  }
#endif
}

/**
 * Prints a detailed representation of a single Concat or BTree cog.
 *
 * @param cog - cog to print
 */
void printTreeCog(struct cog *cog) {
  if (cog->type == COG_CONCAT) {
    printf("U");
  }

  if (cog->type == COG_BTREE) {
#ifndef __ADVANCED
    printf("≤ %ld", cog->data.btree.sep);
#else
    printf("%ld|%ld", cog->data.btree.rds, getReadsAtNode(cog));
#endif
  }
}


/**
 * Prints a detailed representation of a single cog.
 *
 * @param cog - cog to print
 */
void printCog(struct cog *cog) {
  if (cog == NULL) {
    printf("NULL");
  } else {
    if (cog->type == COG_ARRAY || cog->type == COG_SORTEDARRAY) {
      printArrayCog(cog);
    }

    if (cog->type == COG_CONCAT || cog->type == COG_BTREE) {
      printTreeCog(cog);
    }
  }

  printf("\n");
}

/**
 * Prints the internal representation of the JITD providing a detailed layout
 * of the current cogs and data present within.
 *
 * @param cog - the root cog
 * @param depth - depth of the current cog in the tree - set to 0 for root
 */
void printJITD(struct cog *cog, int depth) {
  if (cog == NULL) return;

  if (cog->type == COG_CONCAT || cog->type == COG_BTREE) {
    if (cog->type == COG_CONCAT) {
      if (cog->data.concat.rhs != NULL) {
        printJITD(cog->data.concat.rhs, depth + 1);
      }
    }
    if (cog->type == COG_BTREE) {
      if (cog->data.btree.rhs != NULL) {
        printJITD(cog->data.btree.rhs, depth + 1);
      }
    }

    if (depth != 0) {
      for (int i = 0; i < depth - 1; i++) {
        printf("│   ");
      }
      printf("├───");
    }

    printCog(cog);

    if (cog->type == COG_CONCAT) {
      if (cog->data.concat.lhs != NULL) {
        printJITD(cog->data.concat.lhs, depth + 1);
      }
    }
    if (cog->type == COG_BTREE) {
      if (cog->data.btree.lhs != NULL) {
        printJITD(cog->data.btree.lhs, depth + 1);
      }
    }
  } else {
    if (depth != 0) {
      for (int i = 0; i < depth - 1; i++) {
        printf("│   ");
      }
      printf("├───");
    }

    printCog(cog);
  }
}

#ifdef __ADVANCED
void jsonize(struct cog *cog, FILE *file) {
  if (cog == NULL) fprintf(file, "null");

  if (cog->type == COG_BTREE) {
    fprintf(file, "{\"name\":\"%li ", cog->data.btree.sep);
    fprintf(file, "Total: %li ", cog->data.btree.rds);
    fprintf(file, "Reads: %li\",", getReadsAtNode(cog));
    fprintf(file, "\"children\":[");
    jsonize(cog->data.btree.lhs, file);
    fprintf(file, ",");
    jsonize(cog->data.btree.rhs, file);
    fprintf(file, "]}");
  } else {
    fprintf(file, "{\"name\":\"Elements\"}");
  }
}

/**
 * Converts the JITD to JSON and places it in the file './test.txt'.
 *
 * @param cog - the root cog
 * @param name - output file name
 */
void jsonJITD(struct cog *cog, char *name) {
  FILE *file = fopen(name, "w");
  jsonize(cog, file);
  fclose(file);
}
#endif

/** Prints the current pre-processor mode. */
void printMode() {
#ifndef __ADVANCED
  printf("Classic Mode!\n");
#else
  printf("Advanced Mode!\n");
#endif
}

/**
 * Acquires the count of BTree nodes in a tree.
 *
 * @param cog - the root BTree node in the tree
 * @return the count of BTree nodes in the tree
 */
long getBtreeNodeCount(struct cog *cog) {
  struct cog *left = cog->data.btree.lhs;
  struct cog *right = cog->data.btree.rhs;
  long count = 1;

  if (left != NULL && left->type == COG_BTREE) {
    count += getBtreeNodeCount(left);
  }

  if (right != NULL && right->type == COG_BTREE) {
    count += getBtreeNodeCount(right);
  }

  return count;
}

/**
 * Creates an in-order list of BTree Nodes for a given tree. (step called during recursion).
 *
 * @param cog - root BTree cog
 * @param list - the in-order list
 * @param index - index for next cog in the list
 * @return index for next cog in the list
 */
long inorderStep(struct cog *cog, struct cog **list, long index) {
  struct cog *left = cog->data.btree.lhs;
  struct cog *right = cog->data.btree.rhs;

  if (left != NULL && left->type == COG_BTREE)
    index = inorderStep(left, list, index);

  list[index] = cog;
  index += 1;

  if (right != NULL && right->type == COG_BTREE)
    index = inorderStep(right, list, index);

  return index;
}

/**
 * Creates an in-order list of BTree Nodes for a given tree.
 * NOTE: the in-order list is allocated with malloc, so deallocate with free when done!
 *
 * @param cog - root BTree cog
 * @param count - the count of BTree nodes in the tree
 * @return the in-order list
 */
struct cog **inorder(struct cog *cog, long count) {
  struct cog *left = cog->data.btree.lhs;
  struct cog *right = cog->data.btree.rhs;
  struct cog **list = malloc(count * sizeof(struct cog *));
  long index = 0;

  if (left != NULL && left->type == COG_BTREE)
    index = inorderStep(left, list, index);

  list[index] = cog;
  index += 1;

  if (right != NULL && right->type == COG_BTREE)
    index = inorderStep(right, list, index);

  return list;
}

/**
 * Acquires the median BTree node in a JITD.
 *
 * @param root - the root node of a JITD
 * @return the median node of a JITD
 */
struct cog *getMedian(struct cog *root) {
  long count = getBtreeNodeCount(root);
  struct cog **list = inorder(root, count);
  struct cog *median = list[count/2];
  free(list);
  return median;
}

/**
 * Executes a given function and times the execution.
 *
 * @param function - Function to run
 * @param cog - cog parameter for function
 * @param a - first long parameter for function
 * @param b - second long parameter for function
 * @return the resulting BTree
 */
struct cog *timeRun(struct cog *(*function)(struct cog *, long, long),
                    struct cog *cog,
                    long a,
                    long b) {
  struct timeval stop, start;
  gettimeofday(&start, NULL);
  struct cog *out = (*function)(cog, a, b);
  gettimeofday(&stop, NULL);
  long long startms = start.tv_sec * 1000LL + start.tv_usec / 1000;
  long long stopms = stop.tv_sec * 1000LL + stop.tv_usec / 1000;
  printf("Took %lld milliseconds\n", stopms - startms);
  return out;
}

/**
 * Do a given number of random reads on a cog.
 *
 * @param cog - the given cog
 * @param number - number of reads to do on a cog
 * @param range - the key range for reads
 * @return the resulting BTree
 */
struct cog *randomReads(struct cog *cog, long number, long range) {
  for (long i = 0; i < number; i++) {
    long a = rand() % range;
    long b = rand() % range;
    long low = a <= b ? a : b;
    long high = a > b ? a : b;
    cog = crack(cog, low, high);
  }
  return cog;
}

/**
 * Do a given number of zipfian reads on a cog.
 *
 * @param cog - the given cog
 * @param alpha - zipfian rate of decay
 * @param number - number of reads to do on a cog
 * @param range - the key range for reads
 * @return the resulting BTree
 */
struct cog *zipfianReads(struct cog *cog, double alpha, long number, long range) {
  for (long i = 0; i < number; i++) {
    long a = zipf(alpha, range);
    long b = zipf(alpha, range);
    long low = a <= b ? a : b;
    long high = a > b ? a : b;
    cog = crack(cog, low, high);
  }
  return cog;
}

/**
 * Acquire a random number - no seed issues.
 *
 * @return a random number
 */
int seedlessRandom() {
  static int first = 1;

  if (first == 1) {
    srand(time(NULL) ^ (int)&seedlessRandom);
    first = 0;
  }

  return rand();
}

/**
 * Acquire a random array cog.
 *
 * @param size - size of the array
 * @param range - key range
 * @return a random array cog
 */
cog *getRandomArray(int size, int range) {
  buffer buffer = buffer_alloc(size);

  for(int i = 0; i < size; i++){
    buffer->data[i].key = rand() % range;
    buffer->data[i].value = rand();
  }

  return make_array(0, size, buffer);;
}

#ifdef __HARVEST
/**
 * Run a test involving reads and splaying on a harvested value (last value read).
 *
 * @param cog - the root of the JITD
 * @param reads - number of reads per step
 * @param range - the key range for reads
 * @param doSplay - boolean TRUE or FALSE, if TRUE splay after every step, otherwise just read
 * @param steps - number of steps
 * @return the root of the resulting JITD
 */
struct cog *splayOnHarvest(struct cog *cog, long reads, long range, int doSplay, int steps) {
  for (int i = 0; i < steps; i++) {
    cog = timeRun(randomReads, cog, reads, range);
    if (doSplay != FALSE) {
      cog = splay(cog, getHarvest());
    }
  }
  return cog;
}
#endif

#ifdef __ADVANCED
/**
 * Acquires the cumulative reads at a node if possible.
 *
 * @param cog - a given cog
 * @return the cumulative reads at the cog, 0 if it is NULL or it is not a BTree cog
 */
long getCumulativeReads(struct cog *cog) {
  if (cog == NULL || cog->type != COG_BTREE) return 0;
  return cog->data.btree.rds;
}

/**
 * Acquires the actual read count at a given BTree node.
 *
 * @param cog - a BTree node
 * @return the actual read count for that given BTree node
 */
long getReadsAtNode(struct cog *cog) {
  long count = cog->data.btree.rds;
  if (count == 0) return count;
  count -= getCumulativeReads(cog->data.btree.lhs);
  count -= getCumulativeReads(cog->data.btree.rhs);
  return count;
}
#endif
