
#ifndef _POLICY_UPFRONT_H_SHIELD
#define _POLICY_UPFRONT_H_SHIELD

#ifndef DEFAULT_UPFRONT_LEAF_SIZE
#define DEFAULT_UPFRONT_LEAF_SIZE 1000

template <class Tuple>
class UpfrontIndexPolicy : public RewritePolicyBase <Tuple>
{ 
  std::mutex lock;
  const bool synchInserts;
  const int leafSize;
  
  public:
    CrackerPolicy() : 
      synchInserts(false), leafSize(DEFAULT_UPFRONT_LEAF_SIZE), lock() {}
    CrackerPolicy(bool synchInserts) : 
      synchInserts(synchInserts), lock() {}
    
    std::string name()
      { return std::string("UpfrontIndex"); }
    
    void beforeInsert  (CogHandle<Tuple> root) 
    {
      if(synchInserts) { lock->lock(); }
    }
    void afterInsert   (CogHandle<Tuple> root)
    {
      recurTopDown(std::ref(pushdownArray), root);
      recurTopDown(
        std::bind(inlineArray, leafSize, std::placeholders::_1)
        root
      );
      recurTopDown(
        std::bind(randomSplitOrSortArray, leafSize, std::placeholders::_1)
        root
      );
      recurTopDown(std::ref(balanceBTree), root);
      if(synchInserts) { lock->unlock(); }
    }
    void beforeDelete  (CogHandle<Tuple> root)
    {
      if(synchInserts) { lock->lock(); }
    }
    void afterDelete   (CogHandle<Tuple> root)
    {
      recurTopDown(std::ref(pushdownArray), root);
      recurTopDown(
        std::bind(inlineArray, leafSize, std::placeholders::_1)
        root
      );
      recurTopDown(
        std::bind(randomSplitOrSortArray, leafSize, std::placeholders::_1)
        root
      );
      recurTopDown(std::ref(balanceBTree), root);
      if(synchInserts) { lock->unlock(); }
    }
    void beforeRootIterator(CogHandle<Tuple> node)
    {
      // the JITD is already synchronous.  But, if we're asking
      // for synchronous inserts, we want to wait until organizational
      // tasks are complete.
      if(synchInserts) { lock->lock(); lock->unlock(); }
      lock->lock(); lock->unlock();
    }
    
};

#endif // _POLICY_UPFRONT_H_SHIELD
