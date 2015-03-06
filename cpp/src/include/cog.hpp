#ifndef _COG_H_SHIELD
#define _COG_H_SHIELD

#include <memory>
//#include <atomic>
#include <iostream>

#include "data.hpp"
#include "iterator.hpp"

//class Rewrite;

typedef enum {
  COG_CONCAT,
  COG_BTREE,
  COG_ARRAY,
  COG_SORTED_ARRAY
} CogType;

class Cog {
  
  public:
    Cog(CogType type): type(type) {}
  
    virtual Iterator iterator()
    { 
      std::cerr << "Cog.iterator() is unimplemented" << std::endl;
      exit(-1);
    }
    virtual int size()
    {
      std::cerr << "Cog.size() is unimplemented" << std::endl;
      exit(-1);
    }
//    virtual void rewrite_children(Rewrite &rw)
//    {
//      std::cerr << "Cog.rewrite_children() is unimplemented" << std::endl;
//      exit(-1);
//    }
    
    
    
    void printDebug() { printDebug(0); }
    void prefix(int depth){ while(depth > 0){ std::cout << "  "; depth--; } }
    virtual void printDebug(int depth) {
      prefix(depth);
      std::cout << "???" << std::endl;
    }
    
    CogType type;
};

typedef std::shared_ptr<Cog> CogPtr;

class CogHandleBase {
  /* atomic< */ CogPtr /* > */ ref;
  
  public:
    CogHandleBase(CogPtr init) : ref(init) {}
  
    CogPtr get() { return ref /*.load()*/;  }
    void put(CogPtr &nref) { ref /*.store( */ = nref /*)*/; }
    
    Iterator iterator()                   { return ref->iterator(); }
    int      size()                       { return ref->size(); }
//    void     rewrite_children(Rewrite &rw){ ref->rewrite_children(rw); }
    CogType  type()                       { return ref->type; }
    void     printDebug()                 { ref->printDebug(); }
    void     printDebug(int depth)        { ref->printDebug(depth); }
};

typedef std::shared_ptr<CogHandleBase> CogHandle;

#define MakeHandle(a) CogHandle(new CogHandleBase(shared_ptr<Cog>(a)))


///////////// Cog-Specific Class Headers /////////////

class ConcatCog : public Cog 
{
  public:
    ConcatCog (CogHandle &lhs, CogHandle &rhs) :
      Cog(COG_CONCAT), lhs(lhs), rhs(rhs) {}
  
    CogHandle getLHS(){ return lhs; }
    CogHandle getRHS(){ return rhs; }
    
    Iterator iterator();
    int size(){ return lhs->size() + rhs->size(); }
//    void rewrite_children(Rewrite &rw){ rw.apply(lhs); rw.apply(rhs); }
    
    void printDebug(int depth);
    
  private:
    CogHandle lhs;
    CogHandle rhs;
};

// LHS < k <= RHS
class BTreeCog : public Cog
{
  public:
    BTreeCog (CogHandle &lhs, Key sep, CogHandle &rhs) : 
      Cog(COG_BTREE), lhs(lhs), sep(sep), rhs(rhs) {}
  
    Key getSep(){ return sep; }
    CogHandle getLHS(){ return lhs; }
    CogHandle getRHS(){ return rhs; }
    
    Iterator iterator();
    int size(){ return lhs->size() + rhs->size(); }
//    void rewrite_children(Rewrite &rw){ rw.apply(lhs); rw.apply(rhs); }
//    void rewrite_children(Rewrite &rw, Key target);

    void printDebug(int depth);
    
  private:
    Key sep;
    CogHandle lhs;
    CogHandle rhs;
  
};

class ArrayCog : public Cog 
{
  public:
    ArrayCog(Buffer buffer, BufferElement start, BufferElement end) :
      Cog(COG_ARRAY), buffer(buffer), start(start), end(end) {}
  
    Buffer getBuffer(){ return buffer; }
    BufferElement getStart(){ return start; }
    BufferElement getEnd(){ return end; }
    CogPtr split(Key pivot);

    int size(){ return end-start; }
    Iterator iterator();
//    void rewrite_children(Rewrite &rw){ }

    void printDebug(int depth);
    
  private:
    Buffer buffer;
    BufferElement start;
    BufferElement end;
};

class SortedArrayCog : public Cog 
{
  public:
    SortedArrayCog(Buffer buffer, BufferElement start, BufferElement end) :
      Cog(COG_SORTED_ARRAY), buffer(buffer), start(start), end(end) {}
  
    Buffer getBuffer(){ return buffer; }
    BufferElement getStart(){ return start; }
    BufferElement getEnd(){ return end; }

    int size(){ return end-start; }
//    void rewrite_children(Rewrite &rw){ }
    Iterator iterator();

    void printDebug(int depth);
    
  private:
    Buffer buffer;
    BufferElement start;
    BufferElement end;
};

#endif //_COG_H_SHIELD