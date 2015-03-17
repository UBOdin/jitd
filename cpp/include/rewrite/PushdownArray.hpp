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
          
        } else if((rhs->type == COG_BTREE) && (lhs->type == COG_ARRAY)){
          BTreeCog<Tuple> *btree = (BTreeCog<Tuple> *)rhs.get();
          ArrayCog<Tuple> *arr = (ArrayCog<Tuple> *)lhs.get();
          
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
          
        } else if((rhs->type == COG_BTREE) && (lhs->type == COG_SORTED_ARRAY)){
          BTreeCog<Tuple> *btree = (BTreeCog<Tuple> *)rhs.get();
          SortedArrayCog<Tuple> *arr = (SortedArrayCog<Tuple> *)lhs.get();
          
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
          
        } else if((lhs->type == COG_BTREE) && (rhs->type == COG_BTREE)){
          BTreeCog<Tuple> *blhs = (BTreeCog<Tuple> *)lhs.get();
          BTreeCog<Tuple> *brhs = (BTreeCog<Tuple> *)rhs.get();
          
          if(blhs->sep == brhs->sep){
            h->put(CogPtr<Tuple>(
              new BTreeCog<Tuple>(
                makeHandle(new ConcatCog<Tuple>(blhs->lhs,brhs->lhs)),
                makeHandle(new ConcatCog<Tuple>(blhs->rhs,brhs->rhs)),
                blhs->sep
              )
            ));
            return true;
          }
          if(blhs->sep > brhs->sep){
            BTreeCog<Tuple> *temp = blhs;
            blhs = brhs;
            brhs = temp;
          }
          
          CogPtr<Tuple> lhs_rhs = blhs->rhs->get();
          CogPtr<Tuple> rhs_lhs = brhs->lhs->get();
          
          if((lhs_rhs->type == COG_ARRAY || lhs_rhs->type == COG_SORTED_ARRAY) &&
             (rhs_lhs->type == COG_ARRAY || rhs_lhs->type == COG_SORTED_ARRAY))
          {
            auto lhs_rhs_split = 
              (lhs_rhs->type == COG_ARRAY) ?
                (((ArrayCog<Tuple> *)lhs_rhs.get())->splitCogs(brhs->sep)) :
                (((SortedArrayCog<Tuple> *)lhs_rhs.get())->splitCogs(brhs->sep));
            auto rhs_lhs_split = 
              (rhs_lhs->type == COG_ARRAY) ?
                (((ArrayCog<Tuple> *)rhs_lhs.get())->splitCogs(blhs->sep)) :
                (((SortedArrayCog<Tuple> *)rhs_lhs.get())->splitCogs(blhs->sep));
            
            h->put(CogPtr<Tuple>(
              new BTreeCog<Tuple>(
                makeHandle(new ConcatCog<Tuple>(
                  blhs->lhs,
                  makeHandle(rhs_lhs_split.first)
                )),
                makeHandle(new BTreeCog<Tuple>(
                  makeHandle(new ConcatCog<Tuple>(
                    makeHandle(lhs_rhs_split.first),
                    makeHandle(rhs_lhs_split.second)
                  )),
                  makeHandle(new ConcatCog<Tuple>(
                    makeHandle(lhs_rhs_split.second),
                    brhs->rhs
                  )),
                  brhs->sep
                )),
                blhs->sep
              )
            ));
            return true;
          }
          
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

