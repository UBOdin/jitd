#include <iostream>
#include <vector>

#include "data.hpp"
#include "iterator.hpp"
#include "policy.hpp"

using namespace std;

MergeIterator::MergeIterator(
  CogHandle lhs, CogHandle rhs, RewritePolicy policy
) : policy(policy) 
{
  policy->beforeIterator(lhs);
  lhsIter = lhs->iterator(policy);
  lhsDone = lhsIter->atEnd();
  policy->beforeIterator(rhs);
  rhsIter = rhs->iterator(policy);
  rhsDone = rhsIter->atEnd();
}

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