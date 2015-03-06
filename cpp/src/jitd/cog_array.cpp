#include <iostream>
#include <vector>
#include "cog.hpp"

using namespace std;

Iterator ArrayCog::iterator()
{
  Buffer sorted = Buffer(new vector<Record>(start, end));

  sort(sorted->begin(), sorted->end(), CompareRecord());

  return Iterator(new BufferIterator(sorted));
}
void ArrayCog::printDebug(int depth)
{
  prefix(depth);
  cout << "Array[" << (end-start) << " elements]" << endl;
}
CogPtr ArrayCog::split(Key pivot)
{
  pair<Buffer,Buffer> splits = splitBuffer(start, end, pivot);
  CogPtr lhs(
      new ArrayCog(
        splits.first, 
        splits.first->begin(), 
        splits.first->end()
      ));
  CogPtr rhs(
      new ArrayCog(
        splits.second, 
        splits.second->begin(), 
        splits.second->end()
      ));
  CogHandle lhsH = MakeHandle(lhs);
  CogHandle rhsH = MakeHandle(rhs);
  return CogPtr(new BTreeCog(lhsH, pivot, rhsH));
}