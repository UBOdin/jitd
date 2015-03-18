// Stupid template stupidity hatred burning like the fire of a thousand suns
// forcing include hackery like some sort of stupid stupidity
//
// cog.hpp needs to be processed first, and includes iterator.hpp.  Need to 
// keep this include outside of the shield because of that.
#include "cog.hpp"

#ifndef _ITERATOR_H_SHIELD
#define _ITERATOR_H_SHIELD

// IteratorBase<Tuple>
//
// The primitive superclass for all iterator instances.
//
// Iterators supervise a one-way, ordered traversal of the data defined
// in a cog.  Iterators may be defined recursively, but are expected to behave
// lazily, not materializing any objects until specifically requested to do so
// by a call to get(), next(), or seek().
//
// Ordering is defined by operator<(...) defined for <Tuple>
// 
// An iterator should be initialized pointing to the sequentially first record.
// Thus, a complete scan looks like:
// while(!iter->atEnd()){
//   emit(iter->get());
//   iter->next();
// }

template <class Tuple> 
class IteratorBase {
  public: 
    
    // get()
    // 
    // Retrieve the buffer element that the iterator is currently pointing to.
    // The behavior of get() is undefined if atEnd() == true.
    //
    // When the iterator is first initialized, get() returns the first record.
    virtual BufferElement<Tuple> get()       
      { std::cerr << "Unimplemented Iterator.get()"   << std::endl; exit(-1); }

    // next()
    // 
    // Advance the iterator to the sequentially next (according to 
    // operator<(...)) Tuple.  
    // 
    // The iterator begins initialized.  next() is called *after* get().
    virtual void next()     
      { std::cerr << "Unimplemented Iterator.next()"  << std::endl; exit(-1); }
    
    // seek()
    // 
    // Advance the iterator to the first record lower than or equal to the
    // specified tuple (as per the semantics of lower_bound())
    // 
    // Will never move the iterator backwards.  If k < get(), seek(k) is a no-op
    virtual void seek(const Tuple &k)
      { std::cerr << "Unimplemented Iterator.seek()"  << std::endl; exit(-1); }
      
    // atEnd()
    // 
    // Return true if there are no further tuples 
    virtual bool atEnd()    
      { std::cerr << "Unimplemented Iterator.atEnd()" << std::endl; exit(-1); }
    
    // toBuffer()
    //
    // Construct a sorted buffer from this.
    Buffer<Tuple> toBuffer()
    {
      Buffer<Tuple> ret(new std::vector<Tuple>());
      while(!atEnd()){
        ret->push_back(*get());
        next();
      }
      return ret;
    }
    
    Buffer<Tuple> toBuffer(Tuple &max)
    {
      Buffer<Tuple> ret(new std::vector<Tuple>());
      while(!atEnd() && (*get() < max)){
        ret->push_back(*get());
        next();
      }
      return ret;
    }
    
    Buffer<Tuple> toBuffer(int cnt)
    {
      Buffer<Tuple> ret(new std::vector<Tuple>());
      while(!atEnd() && (--cnt >= 0)){
        ret->push_back(*get());
        next();
      }
      return ret;
    }
    
    // flush()
    //
    // Debug method: Flush all output to the designated ostream
    void flush(std::ostream &o)
    {
      while(!atEnd()){
        o << *get() << " ";
        next();
      }
    }

};
template <class Tuple>
  using Iterator = std::shared_ptr<IteratorBase<Tuple> >;

// BufferIterator
// 
// An implementation of Iterator that traverses a *sorted* Buffer<Tuple>
#include "iterator/BufferIterator.hpp"

// MergeIterator
// 
// An implementation of Iterator that merges the outputs of two Iterators, 
// providing standard iterator semantics over their joint outputs.
#include "iterator/MergeIterator.hpp"

// SeqIterator
// 
// Like MergeIterator, merges the outputs of two iterators into one.  However
// SeqIterator requires a Tuple `sep` such that all values returned by the first
// iterator are < sep, and all values returned by the second iterator are >= 
// sep.
#include "iterator/SeqIterator.hpp"

// DeleteIterator
// 
// Iterator to dynamically delete elements from a stream.  DeleteIterator takes
// two streams: a source stream and a delete stream.  Every tuple in the delete
// stream removes one instance of the corresponding tuple (according to 
// operator==()) from the source stream.
#include "iterator/DeleteIterator.hpp"


#endif // _ITERATOR_H_SHIELD