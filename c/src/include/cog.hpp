#ifndef _COG_H_SHIELD
#define _COG_H_SHIELD

#include <memory>
#include <atomic>

#include "data.hpp"

class CogHandle {
  atomic<shared_ptr<Cog>> ref;
  
  public:
    CogHandle(shared_ptr<Cog> init) : ref(init) {}
  
    shared_ptr<Cog> get() { return ref.load(); }
    swap(shared_ptr<Cog> &nref) { ref.store(nref); }
}

enum CogType {
  COG_CONCAT,
  COG_BTREE,
  COG_ARRAY,
  COG_SORTED_ARRAY
};

class Cog {
  
  public:
    Cog(CogType type): type(type) {}
  
    virtual void *iterator();
    
    void printDebug() { printDebug(0); }
    virtual void printDebug(int depth);
    
    CogType type;
}

class ConcatCog : public Cog 
{
  public:
    ConcatCog(shared_ptr<CogHandle> &lhs, shared_ptr<CogHandle> &rhs) :
      Cog(COG_CONCAT), lhs(lhs), rhs(rhs) {}
  
    shared_ptr<CogHandle> getLHS(){ return lhs; }
    shared_ptr<CogHandle> getRHS(){ return rhs; }
    
    void *iterator();
    void *printDebug(int depth);
    
  private:
    shared_ptr<CogHandle> lhs;
    shared_ptr<CogHandle> rhs;
}

// LHS < k <= RHS
class BTreeCog : public Cog
{
  public:
    ConcatCog(shared_ptr<CogHandle> &lhs, Key sep, shared_ptr<CogHandle> &rhs) :
      Cog(COG_BTREE), lhs(lhs), sep(sep), rhs(rhs) {}
  
    Key getSep(){ return sep; }
    shared_ptr<CogHandle> getLHS(){ return lhs; }
    shared_ptr<CogHandle> getRHS(){ return rhs; }
    
    void *iterator();
    void *printDebug(int depth);
    
  private:
    Key sep;
    shared_ptr<CogHandle> lhs;
    shared_ptr<CogHandle> rhs;
  
}

class ArrayCog : public Cog 
{
  public:
    ArrayCog(Buffer buffer, unsigned int start, unsigned int end) :
      Cog(COG_ARRAY), buffer(buffer), start(start), end(end) {}
  
    Buffer getBuffer(){ return lhs; }
    unsigned int getStart(){ return start; }
    unsigned int getEnd(){ return end; }
    
    void *iterator();
    void *printDebug(int depth);
    
  private:
    shared_ptr<vector<Record>> buffer;
    unsigned int start;
    unsigned int end;
}

class SortedArrayCog : public Cog 
{
  public:
    SortedArrayCog(Buffer buffer, unsigned int start, unsigned int end) :
      Cog(COG_SORTED_ARRAY), buffer(buffer), start(start), end(end) {}
  
    Buffer getBuffer(){ return lhs; }
    unsigned int getStart(){ return start; }
    unsigned int getEnd(){ return end; }
    
    void *iterator();
    void *printDebug(int depth);
    
  private:
    shared_ptr<vector<Record>> buffer;
    unsigned int start;
    unsigned int end;
}


#endif //_COG_H_SHIELD