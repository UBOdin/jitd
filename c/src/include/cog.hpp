#ifndef _COG_H_SHIELD
#define _COG_H_SHIELD

#include <memory>
//#include <atomic>
#include "data.hpp"

class Cog;

class CogHandle {
  /* atomic< */ std::shared_ptr<Cog> /* > */ ref;
  
  public:
    CogHandle(std::shared_ptr<Cog> init) : ref(init) {}
  
    std::shared_ptr<Cog> get() { return ref /*.load()*/;  }
    void swap(std::shared_ptr<Cog> &nref) { ref /*.store( */ = nref /*)*/; }
};

typedef std::shared_ptr<CogHandle> CogHandlePtr;

typedef enum {
  COG_CONCAT,
  COG_BTREE,
  COG_ARRAY,
  COG_SORTED_ARRAY
} CogType;

class Cog {
  
  public:
    Cog(CogType type): type(type) {}
  
    virtual void *iterator();
    
    void printDebug() { printDebug(0); }
    virtual void printDebug(int depth);
    
    CogType type;
};

class ConcatCog : public Cog 
{
  public:
    ConcatCog (
      CogHandlePtr &lhs, 
      CogHandlePtr &rhs
    ) :
      Cog(COG_CONCAT), lhs(lhs), rhs(rhs) {}
  
    CogHandlePtr getLHS(){ return lhs; }
    CogHandlePtr getRHS(){ return rhs; }
    
    void *iterator();
    void printDebug(int depth);
    
  private:
    CogHandlePtr lhs;
    CogHandlePtr rhs;
};

// LHS < k <= RHS
class BTreeCog : public Cog
{
  public:
    BTreeCog (
      CogHandlePtr &lhs, 
      Key sep, 
      CogHandlePtr &rhs
    ) : Cog(COG_BTREE), lhs(lhs), sep(sep), rhs(rhs) {}
  
    Key getSep(){ return sep; }
    CogHandlePtr getLHS(){ return lhs; }
    CogHandlePtr getRHS(){ return rhs; }
    
    void *iterator();
    void printDebug(int depth);
    
  private:
    Key sep;
    CogHandlePtr lhs;
    CogHandlePtr rhs;
  
};

class ArrayCog : public Cog 
{
  public:
    ArrayCog(Buffer buffer, unsigned int start, unsigned int end) :
      Cog(COG_ARRAY), buffer(buffer), start(start), end(end) {}
  
    Buffer getBuffer(){ return buffer; }
    unsigned int getStart(){ return start; }
    unsigned int getEnd(){ return end; }
    
    void *iterator();
    void printDebug(int depth);
    
  private:
    Buffer buffer;
    unsigned int start;
    unsigned int end;
};

class SortedArrayCog : public Cog 
{
  public:
    SortedArrayCog(Buffer buffer, unsigned int start, unsigned int end) :
      Cog(COG_SORTED_ARRAY), buffer(buffer), start(start), end(end) {}
  
    Buffer getBuffer(){ return buffer; }
    unsigned int getStart(){ return start; }
    unsigned int getEnd(){ return end; }
    
    void *iterator();
    void printDebug(int depth);
    
  private:
    Buffer buffer;
    unsigned int start;
    unsigned int end;
};


#endif //_COG_H_SHIELD