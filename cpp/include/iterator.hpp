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
      { std::cerr << "Unimplemented Iterator.get()"   << std::endl; exit(-1); };

};
template <class Tuple>
  using Iterator = std::shared_ptr<IteratorBase<Tuple> >;

#include "iterator/BufferIterator.hpp"
#include "iterator/MergeIterator.hpp"
#include "iterator/SeqIterator.hpp"
#include "iterator/DeleteIterator.hpp"


#endif // _ITERATOR_H_SHIELD