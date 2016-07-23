#ifndef SPLAY_LIB_H_SHIELD
#define SPLAY_LIB_H_SHIELD

#include "cog.h"

#ifdef __ADVANCED
#include "util.h"
#endif


/**
 * The splay operation moves a given node to the root.
 *
 * @param root - current root of the tree
 * @param node - node to be moved to the root
 * @return the new root of the rearranged tree
 */
struct cog *splay(struct cog *root, struct cog *node);

#endif
