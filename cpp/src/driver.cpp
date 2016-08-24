#include <iostream>
#include <fstream>
#include <unistd.h>

#include "jitd.hpp"
#include "test.hpp"

using namespace std;

typedef enum {
  COG_TEST, JITD_TEST
} TestMode;

int main(int argc, char **argv)
{
  istream *src;
  TestMode mode = JITD_TEST;
  int i, t = 0;
  bool interactive;
  JITD<Record> jitd;
  
  srand(time(NULL));
//  sleep(1);
  for(i = 1; i < argc; i++){
    ifstream srcF;
    if((strlen(argv[i]) > 1) && (argv[i][0] == '-')){
      string flag(argv[i]);
      if(string("-c") == flag){ mode = COG_TEST; }
      else if(string("-j") == flag){ mode = JITD_TEST; }
      else {
        cerr << "Invalid command line switch: " << flag << endl;
        exit(-1);
      }
    } else {
      if(string("-") == string(argv[i])){
        src = &cin;
        interactive = true;
      } else {
        srcF.open(argv[i], std::ios_base::in);
        src = &srcF;
        interactive = false;
      }
      switch(mode){
        case COG_TEST:
          cog_test(*src);
          break;
        case JITD_TEST:
          t = jitd_test(jitd, *src, interactive, 0);
          cout << "Time[" << argv[i] << "]: " << t << " s" << endl;
          break;
      }
    }
  }
//  sleep(60);
}
