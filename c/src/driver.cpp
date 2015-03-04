#include <iostream>
#include <fstream>

#include "data.hpp"
#include "cog.hpp"
#include "cog_builder.hpp"

using namespace std;

int main(int argc, char **argv)
{
  ifstream src;
  int i;
  for(i = 1; i < argc; i++){
    src.open(argv[i], std::ios_base::in);
  
    CogHandle ret = build_cog(src);
    ret->printDebug();
  }
}
