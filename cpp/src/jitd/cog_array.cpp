#include <iostream>
#include <vector>
#include "cog.hpp"

using namespace std;


Buffer ArrayCog::sortedBuffer()
{
  Buffer sorted(new vector<Record>(start, end));
  sort(sorted->begin(), sorted->end(), CompareRecord());
  return sorted;
}

Iterator ArrayCog::iterator()
{
  return Iterator(new BufferIterator(sortedBuffer()));
}
void ArrayCog::printDebug(int depth)
{
  prefix(depth);
  cout << "Array[" << (end-start) << " elements]" << endl;
}
pair<Buffer,Buffer> ArrayCog::split(Key pivot)
{
  return splitBuffer(start, end, pivot);
}
CogPtr ArrayCog::splitCog(Key pivot)
{
  pair<Buffer,Buffer> splits = split(pivot);
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
CogPtr ArrayCog::sortedCog()
{
  Buffer sorted = sortedBuffer();
  return CogPtr(new SortedArrayCog(sorted, sorted->begin(), sorted->end()));
}