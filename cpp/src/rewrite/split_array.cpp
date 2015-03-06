
#include <memory>
#include "data.hpp"
#include "rewrite.hpp"
#include "cog.hpp"

using namespace std;

void SplitArrays::apply(CogHandle h)
{
  CogPtr cog = h->get(); // Grab the functional version of this object.
  
  switch(cog->type){
    // Split arrays
    case COG_ARRAY:{
        CogPtr repl = ((ArrayCog *)cog.get())->split(target);
        h->put(repl);
      } break;
    
    // Only recur down the side of BTree Cogs on which target falls
    case COG_BTREE: {
        BTreeCog *bc = (BTreeCog *)cog.get();
        long sep = bc->getSep();
        if(sep > target){ apply(bc->getLHS()); }
        if(sep < target){ apply(bc->getRHS()); }
      } break;
      
    // Default to recurring down all branches
    default: 
      recur(cog);
      break;
  }
}
