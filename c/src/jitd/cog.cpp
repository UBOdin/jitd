#include "cog.hpp"

using namespace std;

Iterator CogHandleBase::iterator() { return ref->iterator(); }
void CogHandleBase::printDebug() { ref->printDebug(); }
void CogHandleBase::printDebug(int depth) { ref->printDebug(depth); }
