#include <iostream>
#include <vector>

#include "data.hpp"
#include "iterator.hpp"
#include "policy.hpp"

using namespace std;


void SeqIterator::initNeeded() { 
  if(!lhsDone) {
    if(lhsIter.get() == NULL) { 
//      cerr << "Initializing LHS" << endl;
      policy->beforeIterator(lhs);
      lhsIter = lhs->iterator(policy);
      lhsDone = lhsIter->atEnd();
    }
  }
  if(lhsDone)  {
    if(rhsIter.get() == NULL) { 
//      cerr << "Initializing RHS" << endl;
      policy->beforeIterator(rhs);
      rhsIter = rhs->iterator(policy);
      rhsDone = rhsIter->atEnd();
    }
  }
}
void SeqIterator::next()
{
  initNeeded();
  if(!lhsDone)     { lhsIter->next(); lhsDone = lhsIter->atEnd(); }
  else if(!rhsDone){ rhsIter->next(); rhsDone = rhsIter->atEnd(); }
}
void SeqIterator::seek(Key k)
{
  if(k >= sep) { lhsDone = true; }
  // check key first... may not need to init LHS.
  initNeeded();
  if(!lhsDone)     { lhsIter->seek(k); lhsDone = lhsIter->atEnd(); }
  else if(!rhsDone){ rhsIter->seek(k); rhsDone = rhsIter->atEnd(); }
}
bool SeqIterator::atEnd()
{
  initNeeded();
  return lhsDone && rhsDone;
}
Key SeqIterator::key()
{
  initNeeded();
  return lhsDone ? rhsIter->key() : lhsIter->key();
}
Value SeqIterator::value()
{
  initNeeded();
  return lhsDone ? rhsIter->value() : lhsIter->value();
}