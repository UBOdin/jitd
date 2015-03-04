#include <iostream>
#include "cog.hpp"

using namespace std;

Iterator ConcatCog::iterator()
{
  return Iterator(new MergeIterator(lhs->iterator(), rhs->iterator()));
}
void ConcatCog::printDebug(int depth)
{
  prefix(depth);
  cout << "Concat" << endl;
  lhs->printDebug(depth+1);
  rhs->printDebug(depth+1);
}
