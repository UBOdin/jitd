#include <iostream>
#include "cog.hpp"

using namespace std;

Iterator BTreeCog::iterator()
{
  return Iterator(new SeqIterator(lhs->iterator(), sep, rhs->iterator()));
}
void BTreeCog::printDebug(int depth)
{
  prefix(depth);
  cout << "BTree[" << sep << "]" << endl;
  lhs->printDebug(depth+1);
  rhs->printDebug(depth+1);
}
