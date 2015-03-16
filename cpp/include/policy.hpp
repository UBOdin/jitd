#ifndef _POLICY_H_SHIELD
#define _POLICY_H_SHIELD

#include "cog.hpp"

template <class Tuple>
class RewritePolicyBase {
  
  public: 
    virtual std::string name()
      { return std::string("Default Policy"); }
    virtual void beforeInsert      (CogHandle<Tuple> root) {}
    virtual void afterInsert       (CogHandle<Tuple> root) {}
    virtual void beforeDelete      (CogHandle<Tuple> root) {}
    virtual void afterDelete       (CogHandle<Tuple> root) {}
    virtual void beforeRootIterator(CogHandle<Tuple> root) {}
    virtual void beforeIterator    (CogHandle<Tuple> node) {}
    virtual void idle              (CogHandle<Tuple> root) {}
    
};

template <class Tuple>
  using RewritePolicy = std::shared_ptr< RewritePolicyBase<Tuple> >;

#define NAIVE_POLICY(type) RewritePolicy<type>(new RewritePolicyBase<type>())




#endif //_POLICY_H_SHIELD