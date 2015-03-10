
#include <iostream>
#include "data.hpp"

using namespace std;

ostream &operator<<(ostream &o, const Record &r)
{
  o << r.key;
  return o;
}
