template <class Tuple> 
class DeleteIterator : public IteratorBase<Tuple> {
  CogHandle<Tuple> source, del;
  Iterator<Tuple> sourceIter, delIter;
  RewritePolicy<Tuple> policy;
  
  public: 
    inline void advanceToNextValidTuple()
    {
      if(!source->atEnd()){ delIter.seek(*sourceIter.get()); }
      while(!source->atEnd() && !del->atEnd() &&
            (*sourceIter.get() == *delIter.get())){
        sourceIter.next();
        delIter.seek(*sourceIter.get());
      }
    }

    DeleteIterator(
      CogHandle<Tuple> source, 
      CogHandle<Tuple> del, 
      RewritePolicy<Tuple> policy
    ) : policy(policy) 
    {
      policy->beforeIterator(source);
      sourceIter = source->iterator(policy);
      policy->beforeIterator(del);
      delIter = del->iterator(policy);
      advanceToNextValidTuple();
    }
    
    void next()
    {
      if(source->atEnd()) { return; }
      source.next();
      advanceToNextValidTuple();
    }
    void seek(const Tuple &k)
    {
      if(source->atEnd()) { return; }
      source.seek(k);
      advanceToNextValidTuple();
    }
    bool atEnd()
    {
      return source->atEnd();
    }
    BufferElement<Tuple> get()
    {
      return source->get();
    }
};