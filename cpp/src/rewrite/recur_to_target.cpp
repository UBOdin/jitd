
#include <memory>
#include "data.hpp"
#include "rewrite.hpp"
#include "cog.hpp"

using namespace std;

void RecurToTarget::apply(CogHandle h)
{
  rw->apply(h);

  CogPtr cog = h->get(); // Grab the functional version of this object.
  switch(cog->type){
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
