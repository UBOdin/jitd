template <class Tuple>
  bool pushdownArray(CogHandle<Tuple> h)
    {
      CogPtr<Tuple> cog = h->get(); // Grab the functional version of this object.
      
      if(cog->type == COG_CONCAT) {
        ConcatCog<Tuple> *concat = ((ConcatCog<Tuple> *)cog.get());
        //Grab functional versions of the kids.
        CogPtr<Tuple> lhs = concat->lhs->get();
        CogPtr<Tuple> rhs = concat->rhs->get();
        
        
        if(lhs->size() == 0){
          h->put(rhs);
          return true;
        } else if(rhs->size() == 0){
          h->put(lhs);
          return true;
        } else if((lhs->type == COG_BTREE) && (rhs->type == COG_ARRAY)){
          BTreeCog<Tuple> *btree = (BTreeCog<Tuple> *)lhs.get();
          ArrayCog<Tuple> *arr = (ArrayCog<Tuple> *)rhs.get();
          
          ArrayCog<Tuple> *lhsArr = NULL;
          ArrayCog<Tuple> *rhsArr = NULL;
          
          std::pair<Buffer<Tuple>,Buffer<Tuple>> split = arr->split(btree->sep);
          if(split.first->size() > 0){
            lhsArr = 
              new ArrayCog<Tuple>(split.first, split.first->begin(), split.first->end());
          }
          if(split.second->size() > 0){
            rhsArr = 
              new ArrayCog<Tuple>(split.second, split.second->begin(), split.second->end());
          }
          
          h->put(CogPtr<Tuple>(
            new BTreeCog<Tuple>(
              (lhsArr == NULL) ? btree->lhs : (
                makeHandle(new ConcatCog<Tuple>(
                  btree->lhs,
                  makeHandle(lhsArr)
                ))
              ),
              (rhsArr == NULL) ? btree->rhs : (
                makeHandle(new ConcatCog<Tuple>(
                  btree->rhs,
                  makeHandle(rhsArr)
                ))
              ),
              btree->sep
            )
          ));
          return true;
          
        } else if((lhs->type == COG_BTREE) && (rhs->type == COG_SORTED_ARRAY)){
          BTreeCog<Tuple> *btree = (BTreeCog<Tuple> *)lhs.get();
          SortedArrayCog<Tuple> *arr = (SortedArrayCog<Tuple> *)rhs.get();
          
          SortedArrayCog<Tuple> *lhsArr = NULL;
          SortedArrayCog<Tuple> *rhsArr = NULL;
          
          BufferElement<Tuple> split = arr->seek(btree->sep);
          if(split - arr->start > 0){
            lhsArr = 
              new SortedArrayCog<Tuple>(arr->buffer, arr->start, split);
          }
          if(arr->end - split > 0){
            rhsArr = 
              new SortedArrayCog<Tuple>(arr->buffer, split, arr->end);
          }
          
          h->put(CogPtr<Tuple>(
            new BTreeCog<Tuple>(
              (lhsArr == NULL) ? btree->lhs : (
                makeHandle(new ConcatCog<Tuple>(
                  btree->lhs,
                  makeHandle(lhsArr)
                ))
              ),
              (rhsArr == NULL) ? btree->rhs : (
                makeHandle(new ConcatCog<Tuple>(
                  btree->rhs,
                  makeHandle(rhsArr)
                ))
              ),
              btree->sep
            )
          ));
          return true;
          
        } 

      } else if(cog->type == COG_DELETE) {
        DeleteCog<Tuple> *delcog = ((DeleteCog<Tuple> *)cog.get());
        //Grab functional versions of the kids.
        CogPtr<Tuple> source = delcog->source->get();
        CogPtr<Tuple> deleted = delcog->deleted->get();
        
        if(deleted->size() == 0){
          h->put(source);
          
        } else if((source->type == COG_BTREE) &&
                  (deleted->type == COG_ARRAY)){
          BTreeCog<Tuple> *btree = (BTreeCog<Tuple> *)source.get();
          ArrayCog<Tuple> *arr = (ArrayCog<Tuple> *)deleted.get();
          
          ArrayCog<Tuple> *lhsArr = NULL;
          ArrayCog<Tuple> *rhsArr = NULL;
          
          std::pair<Buffer<Tuple>,Buffer<Tuple>> split = arr->split(btree->sep);
          if(split.first->size() > 0){
            lhsArr = 
              new ArrayCog<Tuple>(split.first, split.first->begin(), split.first->end());
          }
          if(split.second->size() > 0){
            rhsArr = 
              new ArrayCog<Tuple>(split.second, split.second->begin(), split.second->end());
          }
          
          h->put(CogPtr<Tuple>(
            new BTreeCog<Tuple>(
              (lhsArr == NULL) ? btree->lhs : (
                makeHandle(new DeleteCog<Tuple>(
                  btree->lhs,
                  makeHandle(lhsArr)
                ))
              ),
              (rhsArr == NULL) ? btree->rhs : (
                makeHandle(new DeleteCog<Tuple>(
                  btree->rhs,
                  makeHandle(rhsArr)
                ))
              ),
              btree->sep
            )
          ));
          return true;
                      
        } else if((source->type == COG_BTREE) && 
                  (deleted->type == COG_SORTED_ARRAY)){
          BTreeCog<Tuple> *btree = (BTreeCog<Tuple> *)source.get();
          SortedArrayCog<Tuple> *arr = (SortedArrayCog<Tuple> *)deleted.get();
          
          SortedArrayCog<Tuple> *lhsArr = NULL;
          SortedArrayCog<Tuple> *rhsArr = NULL;
          
          BufferElement<Tuple> split = arr->seek(btree->sep);
          if(split - arr->start > 0){
            lhsArr = 
              new SortedArrayCog<Tuple>(arr->buffer, arr->start, split);
          }
          if(arr->end - split > 0){
            rhsArr = 
              new SortedArrayCog<Tuple>(arr->buffer, split, arr->end);
          }
          
          h->put(CogPtr<Tuple>(
            new BTreeCog<Tuple>(
              (lhsArr == NULL) ? btree->lhs : (
                makeHandle(new DeleteCog<Tuple>(
                  btree->lhs,
                  makeHandle(lhsArr)
                ))
              ),
              (rhsArr == NULL) ? btree->rhs : (
                makeHandle(new DeleteCog<Tuple>(
                  btree->rhs,
                  makeHandle(rhsArr)
                ))
              ),
              btree->sep
            )
          ));
          return true;
          
        } 
        
      } // cog is a delete
      return false;
    } // pushdownArray()

