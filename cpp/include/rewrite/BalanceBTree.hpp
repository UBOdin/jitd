
#ifndef DEFAULT_BTREE_BALANCE_PCT_DIFF
#define DEFAULT_BTREE_BALANCE_PCT_DIFF 10
#endif

#ifndef DEFAULT_BTREE_BALANCE_ABS_DIFF
#define DEFAULT_BTREE_BALANCE_ABS_DIFF 100
#endif

template <class Tuple>
  bool balanceBTree(CogHandle<Tuple> h)
    {
      auto cog = h->get();
      
      if(cog->type == COG_BTREE){
        auto bcog = (BTreeCog<Tuple> *)cog.get();
        auto lhs = bcog->lhs->get();
        auto rhs = bcog->rhs->get();
        
        int lhs_size = lhs->size();
        int rhs_size = rhs->size();
        
        int diff = lhs_size > rhs_size ? lhs_size-rhs_size : rhs_size-lhs_size;
        if(diff < DEFAULT_BTREE_BALANCE_ABS_DIFF) 
          { return false; }
        if(diff * 100 < DEFAULT_BTREE_BALANCE_PCT_DIFF * (lhs_size + rhs_size))
          { return false; }
        
        if(lhs->type == COG_BTREE){
          auto blhs = (BTreeCog<Tuple> *)lhs.get();
          if(blhs->lhs->size() > rhs_size){
            h->put(CogPtr<Tuple>(new BTreeCog<Tuple>(
              blhs->lhs,
              makeHandle(CogPtr<Tuple>(new BTreeCog<Tuple>(
                blhs->rhs,
                bcog->rhs,
                bcog->sep
              ))),
              blhs->sep
            )));
            return true;
          }
        }
        if(rhs->type == COG_BTREE){
          auto brhs = (BTreeCog<Tuple> *)rhs.get();
          if(brhs->rhs->size() > lhs_size){
            h->put(CogPtr<Tuple>(new BTreeCog<Tuple>(
              makeHandle(CogPtr<Tuple>(new BTreeCog<Tuple>(
                bcog->lhs,
                brhs->lhs,
                bcog->sep
              ))),
              brhs->rhs,
              brhs->sep
            )));
            return true;
          }
        }
      }
      return false;
    }
