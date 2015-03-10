#include <iostream>
#include "cog.hpp"
#include "policy.hpp"

using namespace std;

string CrackerPolicy::name() { return string("Cracker"); }
void CrackerPolicy::beforeIterator(CogHandle node)
{
  CogPtr ptr = node->get();
//  cout << "Before Iterator" << endl;
  if(ptr->type == COG_ARRAY){
    ArrayCog *ac = (ArrayCog *)ptr.get();
    if(ac->size() < minSize){
      node->put(ac->sortedCog());
    } else {
      Key splitKey = ac->randKey();
//      std::cerr << "Splitting on " << splitKey << std::endl;
      node->put(ac->splitCog(splitKey));
    }
  }
}