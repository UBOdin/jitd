#ifndef _REWRITE_H_SHIELD
#define _REWRITE_H_SHIELD

#include "cog.hpp"

// rewrite class definition lives in cog.hpp to minimize #include dependency 
// chicanery.

class SplitArray : public Rewrite {
  Key target;
  
  public: 
    SplitArray(Key target): target(target) {}
    void apply(CogHandle &h);
};

#endif //_REWRITE_H_SHIELD