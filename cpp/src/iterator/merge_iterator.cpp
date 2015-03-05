#include <iostream>
#include <vector>

#include "data.hpp"
#include "iterator.hpp"

using namespace std;

void MergeIterator::next()
{
  if(lhsDone && rhsDone) { return; }
  if(lhsBest) { lhs->next(); lhsDone = lhs->atEnd(); }
  else        { rhs->next(); rhsDone = rhs->atEnd(); }
  updateBest();
}
void MergeIterator::seek(Key k)
{
  lhs->seek(k);  lhsDone = lhs->atEnd();
  rhs->seek(k);  rhsDone = rhs->atEnd();
  updateBest();
}
bool MergeIterator::atEnd()
{
  return lhsDone && rhsDone;
}
Key MergeIterator::key()
{
  if(lhsBest) { return lhs->key(); }
  else        { return rhs->key(); }
}
Value MergeIterator::value()
{
  if(lhsBest) { return lhs->value(); }
  else        { return rhs->value(); }
}