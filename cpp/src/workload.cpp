#include <iostream>
#include <set>
#include <vector>

using namespace std;

int main(int argc, char** argv)
{
  int high_mark = 1000*1000*10;
  int low_mark = 1000*1000*1;
  int batch_size = 1000*10;
  int op_count = 1000;
  int max_key = 1000*1000*100;
  vector<int> values;
  
  int diff_mark = high_mark - low_mark;
  int op;
  
  for(op = 0; op < op_count; op++)
  {
    if(op % 100 == 0){ cerr << "." << op; cerr.flush(); }
    if((rand() % diff_mark) > values.size() - low_mark){
      int i;
      cout << "insert explicit";
      for(i = 0; i < batch_size; i++){
        int k = rand() % max_key;
        values.push_back(k);
        cout << " " << k;
      }
      cout << endl;
    } else {
      int i;
      cout << "remove explicit";
      random_shuffle(values.begin(), values.end());
      for(i = 0; i < batch_size; i++){
        cout << " " << values.back();
        values.pop_back();
      }
      cout << endl;
    }
  }
  cerr << endl;
  return 0;
}