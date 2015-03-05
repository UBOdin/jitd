#include <iostream>
#include <vector>

#include "data.hpp"
#include "iterator.hpp"

using namespace std;

void SeqIterator::next()
{
  if(!lhsDone)     { lhs->next(); lhsDone = lhs->atEnd(); }
  else if(!rhsDone){ rhs->next(); rhsDone = rhs->atEnd(); }
}
void SeqIterator::seek(Key k)
{
  if(k >= sep) { lhsDone = true; }
  if(!lhsDone)     { lhs->seek(k); lhsDone = lhs->atEnd(); }
  else if(!rhsDone){ rhs->seek(k); rhsDone = rhs->atEnd(); }
}
bool SeqIterator::atEnd()
{
  return lhsDone && rhsDone;
}
Key SeqIterator::key()
{
  return lhsDone ? rhs->key() : lhs->key();
}
Value SeqIterator::value()
{
  return lhsDone ? rhs->value() : lhs->value();
}