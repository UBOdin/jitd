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
      if(!sourceIter->atEnd() && !delIter->atEnd()){ 
        if(*delIter->get() < *sourceIter->get()){
          delIter->seek(*sourceIter->get()); 
        }
//        std::cerr << "At : " << *sourceIter->get() << "<=>" << *delIter->get() << std::endl;
      }
      while(!sourceIter->atEnd() && !delIter->atEnd() &&
            (*sourceIter->get() == *delIter->get())){
        sourceIter->next();
        delIter->next();
        if(*delIter->get() < *sourceIter->get()){
          delIter->seek(*sourceIter->get());
        }
//        if(!sourceIter->atEnd() && !delIter->atEnd()){
//          std::cerr << "Skip To : " << *sourceIter->get() << "<=>" << *delIter->get() << std::endl;
//        }
      }
    }
};