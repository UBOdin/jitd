#include <iostream>
#include <vector>

#include "data.hpp"
#include "iterator.hpp"

using namespace std;

void BufferIterator::next()
{
  if(curr < end){ curr += 1; }
}
void BufferIterator::seek(Key k)
{
  unsigned int d = 1;
  Record tmp; tmp.key = k; tmp.value = NULL;
  BufferElement high = curr;
  while((high < end) && (high->key < k)){
    curr = high;
    high += d;
    d *= 2;
  }
  curr = lower_bound(curr, high, tmp, CompareRecord());
}
bool BufferIterator::atEnd()
{
  return curr == end;
}
Key BufferIterator::key()
{
  return curr->key;
}
Value BufferIterator::value()
{
  return curr->value;
}