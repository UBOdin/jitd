#ifndef _ITERATOR_H_SHIELD
#define _ITERATOR_H_SHIELD

#include "data.hpp"

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
  Iterator lhs, rhs;
  bool lhsDone, rhsDone;
  bool lhsBest;
  
  public: 
    MergeIterator(Iterator lhs, Iterator rhs) : 
      lhs(lhs), rhs(rhs), lhsDone(lhs->atEnd()), rhsDone(rhs->atEnd()) 
        { updateBest(); };
    
    inline void updateBest() {
      lhsBest = ((!lhsDone) && (rhsDone || (lhs->key() < rhs->key())));
    }

    void next();
    void seek(Key k);
    bool atEnd();
    Key key();
    Value value();
};

class SeqIterator : public IteratorBase {
  Iterator lhs, rhs;
  bool lhsDone, rhsDone;
  Key sep;
  
  public: 
    SeqIterator(Iterator lhs, Key sep, Iterator rhs) : 
      lhs(lhs), rhs(rhs), lhsDone(lhs->atEnd()), rhsDone(rhs->atEnd()), 
      sep(sep) {};

    void next();
    void seek(Key k);
    bool atEnd();
    Key key();
    Value value();
};


#endif // _ITERATOR_H_SHIELD