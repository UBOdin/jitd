#ifndef _JITD_H_SHIELD
#define _JITD_H_SHIELD

#include <iostream>
#include <memory>
#include <atomic>

#include "data.hpp"
#include "cog.hpp"
#include "policy.hpp"
#include "iterator.hpp"
#include "rewrite.hpp"

template<class Tuple>
class JITD {
  
  CogHandle<Tuple>     root;
  RewritePolicy<Tuple> policy;
  
  public:
    JITD() : 
      root(new CogHandleBase<Tuple>(CogPtr<Tuple>(new ArrayCog<Tuple>(
        Buffer<Tuple>(new std::vector<Tuple>()))))),
      policy(new RewritePolicyBase<Tuple>()) 
      {}
    
    CogHandle<Tuple> getRoot()
    {
      return std::atomic_load(root);
    }
    
    Iterator<Tuple> iterator()
    {
      // Root changes with each version, so we need to sync the pointer.
      CogHandle<Tuple> r = std::atomic_load(&root);
      
      // The policy may need to modify the root.  Call it here.
      RewritePolicy<Tuple> p = std::atomic_load(&policy);
      p->beforeRootIterator(r);
      p->beforeIterator(r);
      
      // And grab an iterator on whatever falls out.
      return r->iterator(p);
    }
    
    void insert(Buffer<Tuple> records)
    {
      // Create a template for the new root structure
      ConcatCog<Tuple> *newRootCog = new ConcatCog<Tuple>(
        nullptr,
        CogHandle<Tuple>(new CogHandleBase<Tuple>(CogPtr<Tuple>(
          new ArrayCog<Tuple>(records))))
      );
      // newRootCog->lhs is a placeholder for the current root object
      
      // Define a new root handle to swap in
      CogHandle<Tuple> newRoot = 
        CogHandle<Tuple>(new CogHandleBase<Tuple>(CogPtr<Tuple>(newRootCog)));
      
      // Sync up the policy
      RewritePolicy<Tuple> p = std::atomic_load(&policy);
      
      // Run pre-insertion operations as needed.
      p->beforeInsert(root);
      
      // As one atomic operation, perform:
      //   newRootCog->lhs = root
      //   root = newRoot
      swapInNewRoot(newRoot, newRootCog->lhs);
      
      // Run post-insertion operations as needed.
      p->afterInsert(root);
    }
    
    // remove() exactly mirrors insert.
    void remove(Buffer<Tuple> records)
    {
      DeleteCog<Tuple> *newRootCog = new DeleteCog<Tuple>(
        nullptr,
        CogHandle<Tuple>(new CogHandleBase<Tuple>(CogPtr<Tuple>(
          new ArrayCog<Tuple>(records))))
      );
      CogHandle<Tuple> newRoot = 
        CogHandle<Tuple>(new CogHandleBase<Tuple>(CogPtr<Tuple>(newRootCog)));
      RewritePolicy<Tuple> p = std::atomic_load(&policy);
      p->beforeDelete(root);
      swapInNewRoot(newRoot, newRootCog->source);
      p->afterDelete(newRoot);
    }
    
    void printDebug()
    {
      CogHandle<Tuple> r = std::atomic_load(&root);
      std::cout << "gROOT [" << std::atomic_load(&policy)->name()
                << "; " << r->size() << " elements]" << std::endl;
      r->printDebug(1);
    }
    
    int size()
    {
      return std::atomic_load(&root)->size();
    }
    
    void setPolicy(RewritePolicy<Tuple> newPolicy)
    {
      std::atomic_store(&policy, newPolicy);
    }
    
    RewritePolicy<Tuple> getPolicy()
    {
      return std::atomic_load(&policy);
    }
    
  private: 
    
    // Replace root with newRoot.  placeholder is set to the value of root
    // at time of insertion.  Placeholder may be iterated through several root
    // values until there are no more conflicts.
    inline void swapInNewRoot(
      CogHandle<Tuple> &newRoot, CogHandle<Tuple> &placeholder
    ) {
      bool succeeded = false;
      while(!succeeded){
        placeholder = root;
        succeeded = std::atomic_compare_exchange_strong(
          &root,        // This value gets atomically replaced...
          &placeholder, // ... if this is currently its value
          newRoot       // This value replaces it.
        );
      }
    }
};

#endif //_JITD_H_SHIELD