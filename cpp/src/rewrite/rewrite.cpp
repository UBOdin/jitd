#include "cog.hpp"
#include "rewrite.hpp"

void recurTopDown(Rewrite rw, CogHandle h) 
  { rw(h); recur(makeTopDown(rw), h); }

void recurBottomUp(Rewrite rw, CogHandle h) 
  { recur(makeBottomUp(rw), h); rw(h); }

void recurToTargetTopDown(Rewrite rw, Key target, CogHandle h)
  { rw(h); recurToTarget(makeTargetTopDown(rw, target), target, h); }

void recurToTargetBottomUp(Rewrite rw, Key target, CogHandle h)
  { recurToTarget(makeTargetBottomUp(rw, target), target, h); rw(h); }
