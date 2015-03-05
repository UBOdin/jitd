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
