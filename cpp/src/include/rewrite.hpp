#ifndef _REWRITE_H_SHIELD
#define _REWRITE_H_SHIELD

#include "cog.hpp"

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


#endif //_REWRITE_H_SHIELD