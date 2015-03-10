#ifndef _COG_H_SHIELD
#define _COG_H_SHIELD

#include <memory>
//#include <atomic>
#include <iostream>

#include "data.hpp"

///////////// Forward Refs /////////////

template <class Tuple> 
  class IteratorBase;
template <class Tuple>
  using Iterator = std::shared_ptr<IteratorBase<Tuple> >;

template <class Tuple> 
  class RewritePolicyBase;
template <class Tuple>
  using RewritePolicy = std::shared_ptr<RewritePolicyBase<Tuple> >;

///////////// Global Cog Content /////////////

typedef enum {
  COG_CONCAT,
  COG_BTREE,
  COG_DELETE,
  COG_ARRAY,
  COG_SORTED_ARRAY
} CogType;

template <class Tuple>
class Cog {
  
  public:
    Cog(CogType type): type(type) {}
  
    virtual Iterator<Tuple> iterator(RewritePolicy<Tuple> p)
    { 
      std::cerr << "Cog.iterator() is unimplemented" << std::endl;
      exit(-1);
    }
    virtual int size()
    {
      std::cerr << "Cog.size() is unimplemented" << std::endl;
      exit(-1);
    }    
    
    void printDebug() { printDebug(0); }
    void printPrefix(int depth){ while(depth > 0){ std::cout << "  "; depth--; } }
    virtual void printDebug(int depth) {
      printPrefix(depth);
      std::cout << "???" << std::endl;
    }
    
    CogType type;
};

template <class Tuple>
  using CogPtr = std::shared_ptr< Cog<Tuple> >;

template <class Tuple>
class CogHandleBase {
  /* atomic< */ CogPtr<Tuple> /* > */ ref;
  
  public:
    CogHandleBase(CogPtr<Tuple> init) : ref(init) {}
  
    CogPtr<Tuple> get() { return ref /*.load()*/;  }
    void put(CogPtr<Tuple> nref) { ref /*.store( */ = nref /*)*/; }
    
    Iterator<Tuple> iterator(RewritePolicy<Tuple> p)
                                          { return ref->iterator(p); }
    int             size()                { return ref->size(); }
    CogType         type()                { return ref->type; }
    void            printDebug()          { ref->printDebug(); }
    void            printDebug(int depth) { ref->printDebug(depth); }
};

template <class Tuple>
  using CogHandle = std::shared_ptr<CogHandleBase<Tuple> >;

template <class Tuple>
  inline CogHandle<Tuple> makeHandle(Cog<Tuple> *c){
    return CogHandle<Tuple>(new CogHandleBase<Tuple>(CogPtr<Tuple>(c)));
  }

#include "iterator.hpp"

///////////// Cog-Specific Class Headers /////////////

#include "cog/ConcatCog.hpp"
#include "cog/BTreeCog.hpp"
#include "cog/DeleteCog.hpp"
#include "cog/SortedArrayCog.hpp"
#include "cog/ArrayCog.hpp"

#endif //_COG_H_SHIELD