#ifndef _REWRITE_H_SHIELD
#define _REWRITE_H_SHIELD

#include <memory>
#include <functional>
#include "cog.hpp"

///////////// Global Rewrite Content /////////////

template <class Tuple>
 using Rewrite = std::function<void(CogHandle<Tuple>)>;


///////////// Simple Recurrance /////////////

template <class Tuple>
inline void recur(Rewrite<Tuple> rw, CogPtr<Tuple> cog) 
{
  {
    switch(cog->type)
    {
      case COG_CONCAT: {
          ConcatCog<Tuple> *cc = ((ConcatCog<Tuple> *)cog.get());
          rw(cc->lhs);
          rw(cc->rhs);
        } break;
      
      case COG_BTREE: {
          BTreeCog<Tuple> *bc = ((BTreeCog<Tuple> *)cog.get());
          rw(bc->lhs);
          rw(bc->rhs);
        } break;
      
      case COG_DELETE: {
          DeleteCog<Tuple> *dc = ((DeleteCog<Tuple> *)cog.get());
          rw(dc->source);
          rw(dc->deleted);
        }
      
      case COG_ARRAY:
        break;
        
      case COG_SORTED_ARRAY:
        break;
    }
  }
}
template <class Tuple>
inline void recur(Rewrite<Tuple> rw, CogHandle<Tuple> h) 
  { recur(rw, h->get()); }

///////////// Recur To Target /////////////

template <class Tuple>
inline void recurToTarget(
  Rewrite<Tuple> rw, const Tuple &target, CogPtr<Tuple> cog
){
  {
    switch(cog->type)
    {
      case COG_BTREE: {
          BTreeCog<Tuple> *bc = ((BTreeCog<Tuple> *)cog.get());
          if(target < bc->sep){ rw(bc->lhs); }
          if(target > bc->sep){ rw(bc->rhs); }
        } break;

      default: 
        recur(rw, cog);
    }
  }
}
template <class Tuple>
inline void recurToTarget(
  Rewrite<Tuple> rw, const Tuple &target, CogHandle<Tuple> h
) { recurToTarget(rw, target, h->get()); }

///////////// Parameterized Recursion Rewrites /////////////

template <class Tuple>
  void recurTopDown(Rewrite<Tuple> rw, CogHandle<Tuple> h)
    { 
      recur(
        Rewrite<Tuple>(
          std::bind(recurTopDown<Tuple>, rw, std::placeholders::_1)
        ), h); 
    }

template <class Tuple>
  void recurBottomUp(Rewrite<Tuple> rw, CogHandle<Tuple> h)
    { 
      recur(
        Rewrite<Tuple>(
          std::bind(recurBottomUp<Tuple>, rw, std::placeholders::_1)
        ), h); 
      rw(h); 
    }

template <class Tuple>
  void recurToTargetTopDown(Rewrite<Tuple> rw, const Tuple &target, CogHandle<Tuple> h)
    { 
      rw(h); 
      recurToTarget<Tuple>(
        std::bind(recurToTargetTopDown<Tuple>, rw, target, std::placeholders::_1),
        target, 
        h
      );
    }

template <class Tuple>
  void recurToTargetBottomUp(Rewrite<Tuple> rw, const Tuple &target, CogHandle<Tuple> h)
    { 
      recurToTarget<Tuple>(
        std::bind(recurToTargetBottomUp<Tuple>, rw, target, std::placeholders::_1),
        target, 
        h
      );
      rw(h); 
    }

///////////// Rewrite-Specific Class Headers /////////////

template <class Tuple>
  void splitArray(const Tuple &target, CogHandle<Tuple> h)
    {
      CogPtr<Tuple> cog = h->get(); // Grab the functional version of this object.
      
      if(cog->type == COG_ARRAY) {
        CogPtr<Tuple> repl = ((ArrayCog<Tuple> *)cog.get())->splitCog(target);
        h->put(repl);
      }
    }

template <class Tuple>
  void sortArray(CogHandle<Tuple> h)
    {
      CogPtr<Tuple> cog = h->get(); // Grab the functional version of this object.
      
      if(cog->type == COG_ARRAY){
        CogPtr<Tuple> repl = ((ArrayCog<Tuple> *)cog.get())->sortedCog();
        h->put(repl);
      }
    }

// Concat(BTree(lhs, rhs, sep), arr:Array|SortedArray) -> 
//    -> BTree(Concat(lhs, { x | x in arr, x< sep })
//             Concat(rhs, { x | x in arr, x>=sep }), sep)
// Concat(BTree(lhs, rhs, sep), arr:Array|SortedArray) -> 
//    -> BTree(Concat(lhs, { x | x in arr, x< sep })
//             Concat(rhs, { x | x in arr, x>=sep }), sep)
#include "rewrite/PushdownArray.hpp"

#endif //_REWRITE_H_SHIELD