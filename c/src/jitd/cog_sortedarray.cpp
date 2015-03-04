#include <iostream>
#include "cog.hpp"

using namespace std;

Iterator SortedArrayCog::iterator()
{
  return NULL;
}
void SortedArrayCog::printDebug(int depth)
{
  prefix(depth);
  cout << "SortedArray[" << len << " elements]" << endl;
}
