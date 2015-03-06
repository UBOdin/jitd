
#include <memory>
#include "data.hpp"
#include "rewrite.hpp"
#include "cog.hpp"

using namespace std;

void PushdownArray::apply(CogHandle h)
{
  CogPtr cog = h->get(); // Grab the functional version of this object.
  
  if(cog->type == COG_CONCAT) {
    ConcatCog *concat = ((ConcatCog *)cog.get());
    //Grab functional versions of the kids.
    CogPtr lhs = concat->getLHS()->get();
    CogPtr rhs = concat->getRHS()->get();
    
    if((lhs->type == COG_BTREE) && (rhs->type == COG_ARRAY)){
      BTreeCog *btree = (BTreeCog *)lhs.get();
      ArrayCog *arr = (ArrayCog *)rhs.get();
      
      ArrayCog *lhsArr = NULL;
      ArrayCog *rhsArr = NULL;
      
      pair<Buffer,Buffer> split = arr->split(btree->getSep());
      if(split.first->size() > 0){
        lhsArr = 
          new ArrayCog(split.first, split.first->begin(), split.first->end());
      }
      if(split.second->size() > 0){
        rhsArr = 
          new ArrayCog(split.second, split.second->begin(), split.second->end());
      }
      
      h->put(CogPtr(
        new BTreeCog(
          (lhsArr == NULL) ? btree->getLHS() : (
            MakeHandle(new ConcatCog(
              btree->getLHS(),
              MakeHandle(lhsArr)
            ))
          ),
          btree->getSep(),
          (rhsArr == NULL) ? btree->getRHS() : (
            MakeHandle(new ConcatCog(
              btree->getRHS(),
              MakeHandle(rhsArr)
            ))
          )
        )
      ));
      
    }
    
  }
}
