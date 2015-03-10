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

template <class Tuple>
class ConcatCog : public Cog<Tuple>
{
  public:
    ConcatCog (CogHandle<Tuple> lhs, CogHandle<Tuple> rhs) :
      Cog<Tuple>(COG_CONCAT), lhs(lhs), rhs(rhs) {}
  
    Iterator<Tuple> iterator(RewritePolicy<Tuple> p)
    {
      return Iterator<Tuple>(new MergeIterator<Tuple>(lhs, rhs, p));
    }
    int size(){ return lhs->size() + rhs->size(); }
    
    void printDebug(int depth)
    {
      Cog<Tuple>::printPrefix(depth);
      std::cout << "Concat" << std::endl;
      lhs->printDebug(depth+1);
      rhs->printDebug(depth+1);
    }
    
    const CogHandle<Tuple> lhs;
    const CogHandle<Tuple> rhs;
};

// LHS < k <= RHS
template <class Tuple>
class BTreeCog : public Cog<Tuple>
{
  public:
    BTreeCog (
      CogHandle<Tuple> lhs, 
      CogHandle<Tuple> rhs, 
      Tuple sep
    ) : 
      Cog<Tuple>(COG_BTREE), lhs(lhs), sep(sep), rhs(rhs) {}
  
    Iterator<Tuple> iterator(RewritePolicy<Tuple> p)
    {
      return Iterator<Tuple>(new SeqIterator<Tuple>(lhs, sep, rhs, p));
    }

    int size(){ return lhs->size() + rhs->size(); }

    void printDebug(int depth)
    {
      Cog<Tuple>::printPrefix(depth);
      std::cout << "BTree[" << sep << "]" << std::endl;
      lhs->printDebug(depth+1);
      rhs->printDebug(depth+1);
    }
    const Tuple sep;
    const CogHandle<Tuple> lhs;
    const CogHandle<Tuple> rhs;
  
};

template <class Tuple>
class DeleteCog : public Cog<Tuple>
{
  public:
    DeleteCog (CogHandle<Tuple> source, CogHandle<Tuple> deleted) : 
      Cog<Tuple>(COG_DELETE), source(source), deleted(deleted) {}
  
    CogHandle<Tuple> getDeleted(){ return deleted; }
    CogHandle<Tuple> getSource(){ return source; }
    
    Iterator<Tuple> iterator(RewritePolicy<Tuple> p);
    int size();

    void printDebug(int depth);
    
    const CogHandle<Tuple> source;
    const CogHandle<Tuple> deleted;
  
};

template <class Tuple>
class SortedArrayCog : public Cog<Tuple>
{
  public:
    SortedArrayCog(
      Buffer<Tuple> buffer, 
      BufferElement<Tuple> start, 
      BufferElement<Tuple> end
    ) :
      Cog<Tuple>(COG_SORTED_ARRAY), buffer(buffer), start(start), end(end) {}
  
    Buffer<Tuple> getBuffer(){ return buffer; }
    BufferElement<Tuple> getStart(){ return start; }
    BufferElement<Tuple> getEnd(){ return end; }

    int size(){ return end-start; }
    Iterator<Tuple> iterator(RewritePolicy<Tuple> p)
    {
      return Iterator<Tuple>(new BufferIterator<Tuple>(buffer, start, end));
    }

    void printDebug(int depth)
    {
      Cog<Tuple>::printPrefix(depth);
      std::cout << "SortedArray[" << (end-start) << " elements]" << std::endl;
    }

    
  private:
    Buffer<Tuple> buffer;
    BufferElement<Tuple> start;
    BufferElement<Tuple> end;
};

template <class Tuple>
class ArrayCog : public Cog<Tuple>
{
  public:
    ArrayCog(
      Buffer<Tuple> buffer, 
      BufferElement<Tuple> start, 
      BufferElement<Tuple> end
    ) :
      Cog<Tuple>(COG_ARRAY), buffer(buffer), start(start), end(end) {}
  
    Buffer<Tuple> getBuffer(){ return buffer; }
    Buffer<Tuple> sortedBuffer()
    {
      Buffer<Tuple> sorted(new std::vector<Tuple>(start, end));
      sort(sorted->begin(), sorted->end());
      return sorted;
    }

    std::pair<Buffer<Tuple>,Buffer<Tuple>> split(const Tuple &pivot)
    {
      return splitBuffer(start, end, pivot);
    }
    BufferElement<Tuple> randElement()
      { return (start+(rand() % (end-start))); }
    BufferElement<Tuple> getStart()
      { return start; }
    BufferElement<Tuple> getEnd()
      { return end; }
    CogPtr<Tuple> splitCog(const Tuple &pivot)
    {
      std::pair<Buffer<Tuple>,Buffer<Tuple>> splits = split(pivot);
      CogPtr<Tuple> lhs(
          new ArrayCog<Tuple>(
            splits.first, 
            splits.first->begin(), 
            splits.first->end()
          ));
      CogPtr<Tuple> rhs(
          new ArrayCog<Tuple>(
            splits.second, 
            splits.second->begin(), 
            splits.second->end()
          ));
      CogHandle<Tuple> lhsH(new CogHandleBase<Tuple>(lhs));
      CogHandle<Tuple> rhsH(new CogHandleBase<Tuple>(rhs));
      return CogPtr<Tuple>(new BTreeCog<Tuple>(lhsH, rhsH, pivot));
    }
    CogPtr<Tuple> sortedCog()
    {
      Buffer<Tuple> sorted = sortedBuffer();
      return CogPtr<Tuple>(new SortedArrayCog<Tuple>(
        sorted, sorted->begin(), sorted->end()
      ));
    }

    int size(){ return end-start; }
    Iterator<Tuple> iterator(RewritePolicy<Tuple> p)
    {
      return Iterator<Tuple>(new BufferIterator<Tuple>(sortedBuffer()));
    }

    void printDebug(int depth)
    {
      Cog<Tuple>::printPrefix(depth);
      std::cout << "Array[" << (end-start) << " elements]" << std::endl;
    }
    
  private:
    Buffer<Tuple> buffer;
    BufferElement<Tuple> start;
    BufferElement<Tuple> end;
};

#endif //_COG_H_SHIELD