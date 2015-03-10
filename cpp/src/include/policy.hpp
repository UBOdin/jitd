#ifndef _POLICY_H_SHIELD
#define _POLICY_H_SHIELD

#include "cog.hpp"


class RewritePolicyBase {
  
  public: 
    virtual std::string name();
    virtual void beforeInsert  (CogHandle root);
    virtual void afterInsert   (CogHandle root);
    virtual void beforeIterator(CogHandle node);
    virtual void idle          (CogHandle root);
    
};
typedef std::shared_ptr<RewritePolicyBase> RewritePolicy;

class CrackerPolicy : public RewritePolicyBase 
{ 
  int minSize;
  
  public:
    CrackerPolicy()            : minSize(10) {}
    CrackerPolicy(int minSize) : minSize(minSize) {}
    
    virtual std::string name();
    virtual void beforeIterator(CogHandle node);
    
};

#endif //_POLICY_H_SHIELD