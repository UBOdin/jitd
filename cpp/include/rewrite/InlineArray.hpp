

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
        Buffer<Tuple> newBuff = cog->iterator(NAIVE_POLICY(Tuple))->toBuffer();
        
        if(newBuff->size() != cog->size()){
          std::cerr << "Invalid deletion" << std::endl;
          std::cerr << "Result: ";
          cog->iterator(NAIVE_POLICY(Tuple))->flush(std::cerr);
          std::cerr << std::endl;
          exit(-1);
        }
        
        h->put(
          CogPtr<Tuple>(
            new SortedArrayCog<Tuple>(newBuff)
          )
        );
      }
    
      return rebuild;
    }