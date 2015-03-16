

template <class Tuple>
  bool inlineArray(int bt_merge_threshold, CogHandle<Tuple> h)
    {
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
        h->put(
          CogPtr<Tuple>(
            new SortedArrayCog<Tuple>(
              cog->iterator(NAIVE_POLICY(Tuple))->toBuffer()
            )
          )
        );
      }
    
      return rebuild;
    }