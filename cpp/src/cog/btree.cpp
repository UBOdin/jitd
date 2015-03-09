#include <iostream>
#include "cog.hpp"
#include "iterator.hpp"

using namespace std;

Iterator BTreeCog::iterator(RewritePolicy p)
{
  return Iterator(new SeqIterator(lhs, sep, rhs, p));
}
void BTreeCog::printDebug(int depth)
{
  prefix(depth);
  cout << "BTree[" << sep << "]" << endl;
  lhs->printDebug(depth+1);
  rhs->printDebug(depth+1);
}
