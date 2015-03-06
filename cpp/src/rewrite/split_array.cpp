
#include <memory>
#include "data.hpp"
#include "rewrite.hpp"
#include "cog.hpp"

using namespace std;

void SplitArray::apply(CogHandle &h)
{
  CogPtr c;
  switch(h->type()){
    // Split arrays
    case COG_ARRAY:{
      CogPtr cog = h->get();
      CogPtr repl = ((ArrayCog *)cog.get())->split(target);
      h->put(repl);
      break;
    }
    
    // Only recur down the RHS of BTree Cogs
    case COG_BTREE:
      ((BTreeCog *)h->get().get())->recur(*this, target);
      break;
      
    // Default to recurring down all branches
    default: 
      h->recur(*this);
      break;
  }
}
