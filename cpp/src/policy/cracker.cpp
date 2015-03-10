#include <iostream>
#include "cog.hpp"
#include "policy.hpp"
#include "rewrite.hpp"

using namespace std;

string CrackerPolicy::name() { return string("Cracker"); }
void CrackerPolicy::beforeIterator(CogHandle node)
{
  pushdownArray(node);
  
  CogPtr ptr = node->get();
  if(ptr->type == COG_ARRAY){
    ArrayCog *ac = (ArrayCog *)ptr.get();
    if(ac->size() < minSize){
      node->put(ac->sortedCog());
    } else {
      Key splitKey = ac->randKey();
      node->put(ac->splitCog(splitKey));
    }
  }
}