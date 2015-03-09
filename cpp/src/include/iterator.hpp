#ifndef _ITERATOR_H_SHIELD
#define _ITERATOR_H_SHIELD

#include "data.hpp"
#include "cog.hpp"

class IteratorBase {
  public: 
    virtual void next()     { std::cerr << "Unimplemented Iterator.next()"  << std::endl; exit(-1); }
    virtual void seek(Key k){ std::cerr << "Unimplemented Iterator.seek()"  << std::endl; exit(-1); }
    virtual bool atEnd()    { std::cerr << "Unimplemented Iterator.atEnd()" << std::endl; exit(-1); }
    virtual Key key()       { std::cerr << "Unimplemented Iterator.key()"   << std::endl; exit(-1); };
    virtual Value value()   { std::cerr << "Unimplemented Iterator.value()" << std::endl; exit(-1); }

};
typedef std::shared_ptr<IteratorBase> Iterator;

class BufferIterator : public IteratorBase {
  Buffer buff;
  BufferElement curr, start, end;
  
  public: 
    BufferIterator(Buffer buff) : 
      buff(buff), start(buff->begin()), curr(buff->begin()), end(buff->end()) {}
    BufferIterator(Buffer buff, BufferElement start, BufferElement end) : 
      buff(buff), start(start), curr(start), end(end) {}

    void next();
    void seek(Key k);
    bool atEnd();
    Key key();
    Value value();
};

class MergeIterator : public IteratorBase {
  CogHandle lhs, rhs;
  Iterator lhsIter, rhsIter;
  bool lhsDone, rhsDone;
  bool lhsBest;
  RewritePolicy policy;
  
  public: 
    MergeIterator(CogHandle lhs, CogHandle rhs, RewritePolicy policy) : 
      policy(policy) 
    {
      policy->beforeIterator(lhs);
      lhsIter = lhs->iterator(policy);
      lhsDone = lhsIter->atEnd();
      policy->beforeIterator(rhs);
      rhsIter = rhs->iterator(policy);
      rhsDone = rhsIter->atEnd();
    }
    
    inline void updateBest() {
      lhsBest = ((!lhsDone) && (rhsDone || (lhsIter->key() < rhsIter->key())));
    }

    void next();
    void seek(Key k);
    bool atEnd();
    Key key();
    Value value();
};

class SeqIterator : public IteratorBase {
  CogHandle lhs, rhs;
  Iterator lhsIter, rhsIter;
  bool lhsDone, rhsDone;
  Key sep;
  RewritePolicy policy;
  
  public: 
    SeqIterator(CogHandle lhs, Key sep, CogHandle rhs, RewritePolicy policy) :
      policy(policy), sep(sep), lhs(lhs), rhs(rhs), 
      lhsDone(false), rhsDone(false) {}
    
    inline void initLHS() { if(lhsIter.get() == NULL) { 
      policy->beforeIterator(lhs);
      lhsIter = lhs->iterator(policy);
      lhsDone = lhsIter->atEnd();
    }}
    inline void initRHS() { if(rhsIter.get() == NULL) { 
      policy->beforeIterator(rhs);
      rhsIter = rhs->iterator(policy);
      rhsDone = rhsIter->atEnd();
    }}
    inline void initNeeded() {
      if(!lhsDone) { initLHS(); }
      if(lhsDone)  { initRHS(); }
    }

    void next();
    void seek(Key k);
    bool atEnd();
    Key key();
    Value value();
};


#endif // _ITERATOR_H_SHIELD