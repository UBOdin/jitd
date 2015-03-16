#ifndef _REWRITE_H_SHIELD
#define _REWRITE_H_SHIELD

#include <memory>
#include <functional>
#include "cog.hpp"

///////////// Global Rewrite Content /////////////

template <class Tuple>
 using Rewrite = std::function<bool(CogHandle<Tuple>)>;


///////////// Simple Recurrance /////////////

template <class Tuple>
inline bool recur(Rewrite<Tuple> rw, CogPtr<Tuple> cog) 
{
  {
    bool ret = false;
    switch(cog->type)
    {
      case COG_CONCAT: {
          ConcatCog<Tuple> *cc = ((ConcatCog<Tuple> *)cog.get());
          ret = ret || rw(cc->lhs);
          ret = ret || rw(cc->rhs);
        } break;
      
      case COG_BTREE: {
          BTreeCog<Tuple> *bc = ((BTreeCog<Tuple> *)cog.get());
          ret = ret || rw(bc->lhs);
          ret = ret || rw(bc->rhs);
        } break;
      
      case COG_DELETE: {
          DeleteCog<Tuple> *dc = ((DeleteCog<Tuple> *)cog.get());
          ret = ret || rw(dc->source);
          ret = ret || rw(dc->deleted);
        }
      
      case COG_ARRAY:
        break;
        
      case COG_SORTED_ARRAY:
        break;
    }
    return ret;
  }
}
template <class Tuple>
inline bool recur(Rewrite<Tuple> rw, CogHandle<Tuple> h) 
  { return recur(rw, h->get()); }

///////////// Recur To Target /////////////

template <class Tuple>
inline bool recurToTarget(
  Rewrite<Tuple> rw, const Tuple &target, CogPtr<Tuple> cog
){
  {
    switch(cog->type)
    {
      case COG_BTREE: {
          BTreeCog<Tuple> *bc = ((BTreeCog<Tuple> *)cog.get());
          bool ret = false;
          if(target < bc->sep){ ret = ret || rw(bc->lhs); }
          if(target > bc->sep){ ret = ret || rw(bc->rhs); }
          return ret;
        } break;

      default: 
        return recur(rw, cog);
    }
  }
}
template <class Tuple>
inline bool recurToTarget(
  Rewrite<Tuple> rw, const Tuple &target, CogHandle<Tuple> h
) { return recurToTarget(rw, target, h->get()); }

///////////// Parameterized Recursion Rewrites /////////////

template <class Tuple>
  bool recurTopDown(Rewrite<Tuple> rw, CogHandle<Tuple> h)
    { 
      bool ret = false;
      ret = ret || rw(h);
      ret = ret || recur(
        Rewrite<Tuple>(
          std::bind(recurTopDown<Tuple>, rw, std::placeholders::_1)
        ), h); 
      return ret;
    }

template <class Tuple>
   bool recurBottomUp(Rewrite<Tuple> rw, CogHandle<Tuple> h)
    { 
      bool ret = false;
      ret = ret || recur(
        Rewrite<Tuple>(
          std::bind(recurBottomUp<Tuple>, rw, std::placeholders::_1)
        ), h); 
      ret = ret || rw(h); 
      return ret;
    }

template <class Tuple>
  bool recurToTargetTopDown(Rewrite<Tuple> rw, const Tuple &target, CogHandle<Tuple> h)
    { 
      bool ret = false;
      ret = ret || rw(h); 
      ret = ret || recurToTarget<Tuple>(
        std::bind(recurToTargetTopDown<Tuple>, rw, target, std::placeholders::_1),
        target, 
        h
      );
      return ret;
    }

template <class Tuple>
  bool recurToTargetBottomUp(Rewrite<Tuple> rw, const Tuple &target, CogHandle<Tuple> h)
    { 
      bool ret = false;
      ret = ret || recurToTarget<Tuple>(
        std::bind(recurToTargetBottomUp<Tuple>, rw, target, std::placeholders::_1),
        target, 
        h
      );
      ret = ret || rw(h); 
      return ret;
    }

///////////// Rewrite-Specific Class Headers /////////////

template <class Tuple>
  bool splitArray(const Tuple &target, CogHandle<Tuple> h)
    {
      CogPtr<Tuple> cog = h->get(); // Grab the functional version of this object.
      
      if(cog->type == COG_ARRAY) {
        CogPtr<Tuple> repl = ((ArrayCog<Tuple> *)cog.get())->splitCog(target);
        h->put(repl);
        return repl->type != COG_ARRAY;
      }
      return false;
    }
template <class Tuple>
  bool randomSplitArray(CogHandle<Tuple> h)
    {
      CogPtr<Tuple> cog = h->get(); // Grab the functional version of this object.
      
      if(cog->type == COG_ARRAY) {
        ArrayCog<Tuple> *arr = cog.get();
        CogPtr<Tuple> repl = arr->splitCog(arr->randElement());
        h->put(repl);
        return repl->type != COG_ARRAY;
      }
      return false;
    }

template <class Tuple>
  bool sortArray(CogHandle<Tuple> h)
    {
      CogPtr<Tuple> cog = h->get(); // Grab the functional version of this object.
      
      if(cog->type == COG_ARRAY){
        CogPtr<Tuple> repl = ((ArrayCog<Tuple> *)cog.get())->sortedCog();
        h->put(repl);
        return true;
      }
      return false;
    }
template <class Tuple>
  bool randomSplitOrSortArray(int minSize, CogHandle<Tuple> h)
    {
      CogPtr<Tuple> cog = h->get(); // Grab the functional version of this object.
      
      if(cog->type == COG_ARRAY){
        if(cog->size() > minSize){
          CogPtr<Tuple> repl = ((ArrayCog<Tuple> *)cog.get())->sortedCog();
          h->put(repl);
          return true;
        } else {
          ArrayCog<Tuple> *arr = cog.get();
          CogPtr<Tuple> repl = arr->splitCog(arr->randElement());
          h->put(repl);
          return repl->type != COG_ARRAY;
        }
      }
      return false;
    }
      
      

// pushdownArray()
//
// Push concatenations and deletions down through the tree structure.
//
// Concat(a, b) where a.size() == 0 -> b
// Concat(a, b) where b.size() == 0 -> a
// Concat(BTree(lhs, rhs, sep), arr:Array|SortedArray) -> 
//    -> BTree(Concat(lhs, { x | x in arr, x< sep })
//             Concat(rhs, { x | x in arr, x>=sep }), sep)
// Delete(s, d) where d.size() == 0 -> s
// Delete(BTree(lhs, rhs, sep), arr:Array|SortedArray) -> 
//    -> BTree(Delete(lhs, { x | x in arr, x< sep })
//             Delete(rhs, { x | x in arr, x>=sep }), sep)
#include "rewrite/PushdownArray.hpp"

// inlineArray(bt_merge_threshold)
//
// Inline concatenations and deletions over sorted arrays into a single 
// sorted array.  BTrees are maintained unless the data contained within
// is small enough.
// 
// Concat(a1:SortedArray, a2:SortedArray) -> SortedArray(a1 ++ a2)
// Delete(a1:SortedArray, a2:SortedArray) -> SortedArray(a1 -- a2)
// BTree(a1:SortedArray, a2:SortedArray) 
//    where a1.size() + a2.size() < bt_merge_threshold
//        -> SortedArray(a1 -- a2)
#include "rewrite/InlineArray.hpp"

// balanceBTree()
//
// Pivot BTree nodes so that the two branches of the BTree are more even.
//  - {{A, B}, C} <=> {A, {B, C}}
//    - LHS is better if |C| > |A|, RHS is better if |A| > |C|
//  - {{A, {B, C}}, D} => {{A, B}, {C, D}} <= {A, {{B, C}, D}}
//    - Center is best if D < |B|+|C| > A and D (resp., A) is not a BTree node.
//    - if D/A is a BTree node, the first rule works.
//  
// 
// BTree(BTree(a, b, sep1), c, sep2) where a.size() > c.size() 
//        -> BTree(a, BTree(b, c, sep2), sep1)
// BTree(a, BTree(b, c, sep1), sep2) where a.size() < c.size() 
//        -> BTree(BTree(a, b, sep2), c, sep1)
#include "rewrite/BalanceBTree.hpp"


#endif //_REWRITE_H_SHIELD