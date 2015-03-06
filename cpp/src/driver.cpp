#include <iostream>
#include <fstream>
#include <unistd.h>

#include "data.hpp"
#include "cog.hpp"
#include "cog_tester.hpp"

using namespace std;

int main(int argc, char **argv)
{
  ifstream src;
  int i;
  srand(time(NULL));
//  sleep(1);
  for(i = 1; i < argc; i++){
    src.open(argv[i], std::ios_base::in);
  
    cog_test(src);
  }
//  sleep(60);
}
