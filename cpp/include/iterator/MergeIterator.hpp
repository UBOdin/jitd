template <class Tuple> 
class MergeIterator : public IteratorBase<Tuple> {
  CogHandle<Tuple> lhs, rhs;
  Iterator<Tuple> lhsIter, rhsIter;
  bool lhsDone, rhsDone;
  bool lhsBest;
  RewritePolicy<Tuple> policy;
  
  public: 
    MergeIterator(
      CogHandle<Tuple> lhs, 
      CogHandle<Tuple> rhs, 
      RewritePolicy<Tuple> policy
    ) : policy(policy) 
    {
      policy->beforeIterator(lhs);
      lhsIter = lhs->iterator(policy);
      lhsDone = lhsIter->atEnd();
      policy->beforeIterator(rhs);
      rhsIter = rhs->iterator(policy);
      rhsDone = rhsIter->atEnd();
      updateBest();
    }
    
    inline void updateBest() 
    {
      lhsBest = ((!lhsDone) && (rhsDone || (*lhsIter->get() < *rhsIter->get())));
//      std::cerr << "Cmp: " << *lhsIter->get() << " ^^ " << *rhsIter->get() << std::endl;
    }

    void next()
    {
      if(lhsDone && rhsDone) { return; }
      if(lhsBest) { lhsIter->next(); lhsDone = lhsIter->atEnd(); }
      else        { rhsIter->next(); rhsDone = rhsIter->atEnd(); }
      updateBest();
    }
    void seek(const Tuple &k)
    {
      lhsIter->seek(k);  lhsDone = lhsIter->atEnd();
      rhsIter->seek(k);  rhsDone = rhsIter->atEnd();
      updateBest();
    }
    bool atEnd()
    {
      return lhsDone && rhsDone;
    }
    BufferElement<Tuple> get()
    {
      if(lhsBest) { return lhsIter->get(); }
      else        { return rhsIter->get(); }
    }
};