#include <iostream>
#include <fstream>

#include "data.hpp"
#include "cog.hpp"
#include "cog_builder.hpp"

using namespace std;

int main(int argc, char **argv){
  ifstream src;
  src.open("test/array1.cog", std::ios_base::in);
  
  CogHandle ret = build_cog(src);
  
}
