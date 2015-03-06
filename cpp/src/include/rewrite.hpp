#ifndef _REWRITE_H_SHIELD
#define _REWRITE_H_SHIELD

#include "cog.hpp"

class Rewrite {
  public: 
    virtual void apply(CogHandle h);
    void recur(CogHandle h);
};

class SplitArray : public Rewrite {
  Key target;
  
  public: 
    SplitArray(Key target): target(target) {}
    void apply(CogHandle h);
};

#endif //_REWRITE_H_SHIELD