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
    
    cout << "Size: " << ret->size() << " records" << endl;
    cout << "---------------" << endl;
    
    Iterator iter = ret->iterator();
    int row = 1;
    while(!iter->atEnd()){
      cout << row << " -> " << iter->key() << endl;
      iter->next();
      row++;
    }
    cout << "Total: " << (row-1) << " records" << endl;
  }
}
