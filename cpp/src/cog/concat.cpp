#include <iostream>
#include "cog.hpp"
#include "iterator.hpp"

using namespace std;

Iterator ConcatCog::iterator(RewritePolicy p)
{
  return Iterator(new MergeIterator(lhs, rhs, p));
}
void ConcatCog::printDebug(int depth)
{
  prefix(depth);
  cout << "Concat" << endl;
  lhs->printDebug(depth+1);
  rhs->printDebug(depth+1);
}
