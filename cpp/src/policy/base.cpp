

#include <iostream>
#include "cog.hpp"
#include "policy.hpp"

using namespace std;

string RewritePolicyBase::name() { return string("Default Policy"); }
void RewritePolicyBase::beforeInsert  (CogHandle root) {}
void RewritePolicyBase::afterInsert   (CogHandle root) {}
void RewritePolicyBase::beforeIterator(CogHandle node) {std::cout << "FOO" <<std::endl;}
void RewritePolicyBase::idle          (CogHandle root) {}
