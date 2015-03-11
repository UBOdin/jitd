template <class Tuple> 
class DeleteIterator : public IteratorBase<Tuple> {
  CogHandle<Tuple> source, del;
  Iterator<Tuple> sourceIter, delIter;
  RewritePolicy<Tuple> policy;
  
  public: 

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
      advanceWhileCurrentTupleIsInvalid();
    }
    
    void next()
    {
      if(sourceIter->atEnd()) { return; }
      sourceIter->next();
      advanceWhileCurrentTupleIsInvalid();
    }
    void seek(const Tuple &k)
    {
      if(sourceIter->atEnd()) { return; }
      sourceIter->seek(k);
      advanceWhileCurrentTupleIsInvalid();
    }
    bool atEnd()
    {
      return sourceIter->atEnd();
    }
    BufferElement<Tuple> get()
    {
      return sourceIter->get();
    }
  
  private:
  
    inline void advanceWhileCurrentTupleIsInvalid()
    {
      if(!sourceIter->atEnd()){ delIter->seek(*sourceIter->get()); }
      while(!sourceIter->atEnd() && !sourceIter->atEnd() &&
            (*sourceIter->get() == *delIter->get())){
        sourceIter->next();
        delIter->seek(*sourceIter->get());
      }
    }
};