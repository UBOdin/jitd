#ifndef _REWRITE_H_SHIELD
#define _REWRITE_H_SHIELD

#include "cog.hpp"

// this all should probably be rewritten with bind() or equivalent.

///////////// Global Rewrite Content /////////////

class Rewrite {
  public: 
    virtual void apply(CogHandle h);
    inline void recur(CogHandle h)
      {
        recur(h->get());
      }
    inline void recur(CogPtr cog)
      {
        switch(cog->type)
        {
          case COG_CONCAT: {
              ConcatCog *cc = ((ConcatCog *)cog.get());
              apply(cc->getLHS());
              apply(cc->getRHS());
            } break;
          
          case COG_BTREE: {
              BTreeCog *bc = ((BTreeCog *)cog.get());
              apply(bc->getLHS());
              apply(bc->getRHS());
            } break;
          
          case COG_ARRAY:
            break;
            
          case COG_SORTED_ARRAY:
            break;
        }
      }
};

///////////// Parameterized Recursion Rewrites /////////////

class RecurTopDown : public Rewrite {
  std::unique_ptr<Rewrite> rw;
  
  public:
    RecurTopDown(Rewrite *rw): rw(rw) {}
    void apply(CogHandle h) { rw->apply(h); recur(h); }
};

class RecurBottomUp : public Rewrite {
  std::unique_ptr<Rewrite> rw;
  
  public:
    RecurBottomUp(Rewrite *rw): rw(rw) {}
    void apply(CogHandle h) { recur(h); rw->apply(h); }
};

class RecurToTarget : public Rewrite {
  std::unique_ptr<Rewrite> rw;
  Key target;
  
  public:
    RecurToTarget(Rewrite *rw, Key target): rw(rw), target(target) {}
    void apply(CogHandle h);
};

///////////// Rewrite-Specific Class Headers /////////////

class SplitArrays : public Rewrite {
  Key target;
  
  public: 
    SplitArrays(Key target): target(target) {}
    void apply(CogHandle h);
};

class SortArrays : public Rewrite {
  public: 
    SortArrays() {}
    void apply(CogHandle h);
};

class PushdownArray : public Rewrite {
  public: 
    PushdownArray() {}
    void apply(CogHandle h);
};


#endif //_REWRITE_H_SHIELD