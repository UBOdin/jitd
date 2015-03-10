// Stupid template stupidity hatred burning like the fire of a thousand suns
// forcing include hackery like some sort of stupid stupidity
#include "cog.hpp"

#ifndef _ITERATOR_H_SHIELD
#define _ITERATOR_H_SHIELD



template <class Tuple> 
class IteratorBase {
  public: 
    virtual void next()     
      { std::cerr << "Unimplemented Iterator.next()"  << std::endl; exit(-1); }
    virtual void seek(const Tuple &k)
      { std::cerr << "Unimplemented Iterator.seek()"  << std::endl; exit(-1); }
    virtual bool atEnd()    
      { std::cerr << "Unimplemented Iterator.atEnd()" << std::endl; exit(-1); }
    virtual BufferElement<Tuple> get()       
      { std::cerr << "Unimplemented Iterator.curr()"   << std::endl; exit(-1); };

};
template <class Tuple>
  using Iterator = std::shared_ptr<IteratorBase<Tuple> >;

template <class Tuple> 
class BufferIterator : public IteratorBase<Tuple> {
  Buffer<Tuple> buff;
  BufferElement<Tuple> curr, start, end;
  
  public: 
    BufferIterator(Buffer<Tuple> buff) : 
      buff(buff), start(buff->begin()), curr(buff->begin()), end(buff->end()) {}
    BufferIterator(
      Buffer<Tuple> buff, 
      BufferElement<Tuple> start, 
      BufferElement<Tuple> end
    ) : 
      buff(buff), start(start), curr(start), end(end) {}

    void next()
    {
      if(curr < end){ curr += 1; }
    }

    void seek(const Tuple &k)
    {
      unsigned int d = 1;
      BufferElement<Tuple> high = curr;
      while((high < end) && (*high < k)){
        curr = high;
        high += d;
        d *= 2;
      }
      curr = lower_bound(curr, high, k);
    }

    bool atEnd()
    {
      return curr == end;
    }

    BufferElement<Tuple> get()
    {
      return curr;
    }
};

template <class Tuple> 
class MergeIterator : public IteratorBase<Tuple> {
  CogHandle<Tuple> lhs, rhs;
  Iterator<Tuple> lhsIter, rhsIter;
  bool lhsDone, rhsDone;
  bool lhsBest;
  RewritePolicy<Tuple> policy;
  
  public: 
    MergeIterator(
      CogHandle<Tuple> lhs, 
      CogHandle<Tuple> rhs, 
      RewritePolicy<Tuple> policy
    ) : policy(policy) 
    {
      policy->beforeIterator(lhs);
      lhsIter = lhs->iterator(policy);
      lhsDone = lhsIter->atEnd();
      policy->beforeIterator(rhs);
      rhsIter = rhs->iterator(policy);
      rhsDone = rhsIter->atEnd();
    }
    
    inline void updateBest() 
    {
      lhsBest = ((!lhsDone) && (rhsDone || (lhsIter->get() < rhsIter->get())));
    }

    void next()
    {
      if(lhsDone && rhsDone) { return; }
      if(lhsBest) { lhsIter->next(); lhsDone = lhsIter->atEnd(); }
      else        { rhsIter->next(); rhsDone = rhsIter->atEnd(); }
      updateBest();
    }
    void seek(const Tuple &k)
    {
      lhsIter->seek(k);  lhsDone = lhsIter->atEnd();
      rhsIter->seek(k);  rhsDone = rhsIter->atEnd();
      updateBest();
    }
    bool atEnd()
    {
      return lhsDone && rhsDone;
    }
    BufferElement<Tuple> get()
    {
      if(lhsBest) { return lhsIter->get(); }
      else        { return rhsIter->get(); }
    }
};

template <class Tuple> 
class SeqIterator : public IteratorBase<Tuple> {
  CogHandle<Tuple> lhs, rhs;
  Iterator<Tuple> lhsIter, rhsIter;
  bool lhsDone, rhsDone;
  const Tuple sep;
  RewritePolicy<Tuple> policy;
  
  public: 
    SeqIterator(
      CogHandle<Tuple> lhs, 
      Tuple sep, 
      CogHandle<Tuple> rhs, 
      RewritePolicy<Tuple> policy
    ) :
      policy(policy), sep(sep), lhs(lhs), rhs(rhs), 
      lhsDone(false), rhsDone(false) {}
    
    inline void initNeeded()
    { 
      if(!lhsDone) {
        if(lhsIter.get() == NULL) { 
          policy->beforeIterator(lhs);
          lhsIter = lhs->iterator(policy);
          lhsDone = lhsIter->atEnd();
        }
      }
      if(lhsDone)  {
        if(rhsIter.get() == NULL) { 
          policy->beforeIterator(rhs);
          rhsIter = rhs->iterator(policy);
          rhsDone = rhsIter->atEnd();
        }
      }
    }

    void next()
    {
      initNeeded();
      if(!lhsDone)     { lhsIter->next(); lhsDone = lhsIter->atEnd(); }
      else if(!rhsDone){ rhsIter->next(); rhsDone = rhsIter->atEnd(); }
    }

    void seek(const Tuple &k)
    {
      if(!(k < sep)) { lhsDone = true; }
      // check key first... may not need to init LHS.
      initNeeded();
      if(!lhsDone)     { lhsIter->seek(k); lhsDone = lhsIter->atEnd(); }
      else if(!rhsDone){ rhsIter->seek(k); rhsDone = rhsIter->atEnd(); }
    }

    bool atEnd()
    {
      initNeeded();
      return lhsDone && rhsDone;
    }
    
    BufferElement<Tuple> get()
    {
      initNeeded();
      return lhsDone ? rhsIter->get() : lhsIter->get();
    }
};


#endif // _ITERATOR_H_SHIELD