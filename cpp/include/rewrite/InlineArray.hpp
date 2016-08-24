

template <class Tuple>
  bool inlineArray(
    int bt_merge_threshold, 
    int bt_split_threshold, 
    CogHandle<Tuple> h
  ){
      auto cog = h->get();
      bool rebuild = false;
      
      
      switch(cog->type) {
        case COG_CONCAT: 
          rebuild = (((ConcatCog<Tuple>*)cog.get())->lhs->type() == COG_SORTED_ARRAY)
                 && (((ConcatCog<Tuple>*)cog.get())->rhs->type() == COG_SORTED_ARRAY);
          break;
        
        case COG_DELETE: 
          rebuild = (((DeleteCog<Tuple>*)cog.get())->source->type() == COG_SORTED_ARRAY)
                 && (((DeleteCog<Tuple>*)cog.get())->deleted->type() == COG_SORTED_ARRAY);
          break;
        
        case COG_BTREE: 
          rebuild = (cog->size() < bt_merge_threshold);
          break;
        
        case COG_SORTED_ARRAY:
        case COG_ARRAY:
          rebuild = false;
          break;
      }
      
      if(rebuild){
        if((bt_split_threshold <= 0) || (cog->size() < bt_split_threshold)){
          Buffer<Tuple> newBuff = cog->iterator(NAIVE_POLICY(Tuple))->toBuffer();
          
          h->put(
            CogPtr<Tuple>(
              new SortedArrayCog<Tuple>(newBuff)
            )
          );
        } else {
          Iterator<Tuple> i = cog->iterator(NAIVE_POLICY(Tuple));
          Buffer<Tuple> newBuff = i->toBuffer(bt_split_threshold);
          CogPtr<Tuple> root(
              new SortedArrayCog<Tuple>(newBuff)
            );
          while(!i->atEnd()){
            BufferElement<Tuple> sep = i->get();
            newBuff = i->toBuffer(bt_split_threshold);
            CogPtr<Tuple> oldRoot = root;
            root = CogPtr<Tuple>(
              new BTreeCog<Tuple>(
                CogHandle<Tuple>(new CogHandleBase<Tuple>(oldRoot)),
                makeHandle(new SortedArrayCog<Tuple>(newBuff)),
                *sep
              )
            );
          }
          h->put(root);
        }
      }
    
      return rebuild;
    }


template <class Tuple>
  bool inlineArray(int bt_target_size, CogHandle<Tuple> h) {
    return inlineArray(bt_target_size, bt_target_size, h);
  }