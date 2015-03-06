
#include <memory>
#include "data.hpp"
#include "rewrite.hpp"
#include "cog.hpp"

using namespace std;

void SortArrays::apply(CogHandle h)
{
  CogPtr cog = h->get(); // Grab the functional version of this object.
  
  switch(cog->type){
    // Sort arrays
    case COG_ARRAY:{
        cout << "Sorting" <<endl;
        CogPtr repl = ((ArrayCog *)cog.get())->sortedCog();
        h->put(repl);
      } break;
      
    // Default to recurring down all branches
    default: 
      recur(cog);
      break;
  }
}
