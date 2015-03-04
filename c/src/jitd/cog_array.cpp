#include <iostream>
#include "cog.hpp"

using namespace std;

Iterator ArrayCog::iterator()
{
  return NULL;
}
void ArrayCog::printDebug(int depth)
{
  prefix(depth);
  cout << "Array[" << len << " elements]" << endl;
}
