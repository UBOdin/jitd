#include <iostream>
#include <vector>

#include "data.hpp"
#include "iterator.hpp"

using namespace std;

void MergeIterator::next()
{
  if(lhsDone && rhsDone) { return; }
  if(lhsBest) { lhsIter->next(); lhsDone = lhsIter->atEnd(); }
  else        { rhsIter->next(); rhsDone = rhsIter->atEnd(); }
  updateBest();
}
void MergeIterator::seek(Key k)
{
  lhsIter->seek(k);  lhsDone = lhsIter->atEnd();
  rhsIter->seek(k);  rhsDone = rhsIter->atEnd();
  updateBest();
}
bool MergeIterator::atEnd()
{
  return lhsDone && rhsDone;
}
Key MergeIterator::key()
{
  if(lhsBest) { return lhsIter->key(); }
  else        { return rhsIter->key(); }
}
Value MergeIterator::value()
{
  if(lhsBest) { return lhsIter->value(); }
  else        { return rhsIter->value(); }
}