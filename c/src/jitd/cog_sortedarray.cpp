#include <iostream>
#include "cog.hpp"

using namespace std;

Iterator SortedArrayCog::iterator()
{
  return Iterator(new BufferIterator(buffer, start, end));
}
void SortedArrayCog::printDebug(int depth)
{
  prefix(depth);
  cout << "SortedArray[" << (end-start) << " elements]" << endl;
}
