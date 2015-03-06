
#include "rewrite.hpp"
#include "cog.hpp"

using namespace std;

class SplitArray : public Rewrite {
  Key target;
  
  public: 
    SplitArray(Key target): target(target) {}
    
    operation()(CogHandle h)
    {
      CogPtr c;
      switch(h->type()){
        case COG_ARRAY:{
          CogPtr cog = h->get();
          h->put(split((ArrayCog *)cog.get()));
          break;
        }

        case COG_BTREE:{
          CogPtr cog = h->get();
          BTreeCog btreecog = ((BTreeCog *)cog.get())
          Key sep = btreecog->getSep();
          if(sep == target){ break; }
          if(sep > target){ btreecog->getLHS()->recur(this); }
          else            { btreecog->getRHS()->recur(this); }
          break;
        }
          
        default: 
          h->recur(this);
          break;
      }
    }

}