#ifndef _REWRITE_H_SHIELD
#define _REWRITE_H_SHIELD

#include <memory>
#include <functional>
#include "cog.hpp"

///////////// Global Rewrite Content /////////////

typedef std::function<void(CogHandle)> Rewrite;


///////////// Simple Recurrance /////////////

inline void recur(Rewrite rw, CogPtr cog) 
{
  {
    switch(cog->type)
    {
      case COG_CONCAT: {
          ConcatCog *cc = ((ConcatCog *)cog.get());
          rw(cc->getLHS());
          rw(cc->getRHS());
        } break;
      
      case COG_BTREE: {
          BTreeCog *bc = ((BTreeCog *)cog.get());
          rw(bc->getLHS());
          rw(bc->getRHS());
        } break;
      
      case COG_ARRAY:
        break;
        
      case COG_SORTED_ARRAY:
        break;
    }
  }
}
inline void recur(Rewrite rw, CogHandle h) 
  { recur(rw, h->get()); }

///////////// Recur To Target /////////////

inline void recurToTarget(Rewrite rw, Key target, CogPtr cog) 
{
  {
    switch(cog->type)
    {
      case COG_BTREE: {
          BTreeCog *bc = ((BTreeCog *)cog.get());
          Key sep = bc->getSep();
          if(sep > target){ rw(bc->getLHS()); }
          if(sep < target){ rw(bc->getRHS()); }
        } break;

      default: 
        recur(rw, cog);
    }
  }
}
inline void recurToTarget(Rewrite rw, Key target, CogHandle h) 
  { recurToTarget(rw, target, h->get()); }

///////////// Parameterized Recursion Rewrites /////////////

void recurTopDown(Rewrite rw, CogHandle h);
inline Rewrite makeTopDown(Rewrite rw)
  { return std::bind(recurTopDown, rw, std::placeholders::_1); }

void recurBottomUp(Rewrite rw, CogHandle h);
inline Rewrite makeBottomUp(Rewrite rw)
  { return std::bind(recurBottomUp, rw, std::placeholders::_1); }

void recurToTargetTopDown(Rewrite rw, Key target, CogHandle h);
inline Rewrite makeTargetTopDown(Rewrite rw, Key target)
  { return std::bind(recurToTargetTopDown, rw, target, std::placeholders::_1); }

void recurToTargetBottomUp(Rewrite rw, Key target, CogHandle h);
inline Rewrite makeTargetBottomUp(Rewrite rw, Key target)
  { return std::bind(recurToTargetBottomUp, rw, target, std::placeholders::_1); }

///////////// Rewrite-Specific Class Headers /////////////

void splitArray(Key target, CogHandle h);
inline Rewrite makeSplitArray(Key target) 
  { return std::bind(splitArray, target, std::placeholders::_1); }

void sortArray(CogHandle h);
inline Rewrite makeSortArray() 
  { return Rewrite(sortArray); }

void pushdownArray(CogHandle h);
inline Rewrite makePushdownArray() 
  { return Rewrite(pushdownArray); }


#endif //_REWRITE_H_SHIELD