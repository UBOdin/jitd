#include <iostream>
#include <set>
#include <vector>

using namespace std;


int main(int argc, char** argv)
{
  long int high_mark = 1000*1000*1000*10;
  long int low_mark = 1000*1000*1000*1;
  int batch_size = 1000*1000*100;
  long int op_count = 100;
  int max_key = 1000*1000*100;
  vector< vector<int> > values;
  
  long int diff_mark = high_mark - low_mark;
  long int op;
  
  srand(time(NULL));
  
  for(op = 0; op < op_count; op++)
  {
//    if(op % 10 == 0){ 
      cerr <<endl<< ":" << op; cerr.flush(); 
//      }
    long int choice = rand() % diff_mark;
    long int threshold = values.size() - low_mark;
    char cmd;
    int cnt;
    if(choice > threshold){
      int i;
      cmd = 'i';
      cnt = batch_size;
      cout.write((char *)&cmd, sizeof(cmd));
      cout.write((char *)&cnt, sizeof(cnt));
      
      values.emplace_back(cnt);
      
      for(i = 0; i < cnt; i++){
        if(i % (1000*1000) == 0){
          cerr << "." << i; cerr.flush();
        }
        int k = rand() % max_key;
        values.back().push_back(k);
        cout.write((char *)&k, sizeof(k));
      }
      random_shuffle(values.back().begin(), values.back().end());
      cout << endl;
    } else {
      int i;
      cmd = 'r';
      cnt = batch_size;
      cout.write((char *)&cmd, sizeof(cmd));
      cout.write((char *)&cnt, sizeof(cnt));
      for(i = 0; i < cnt; i++){
        if(i % (1000*1000) == 0){
          cerr << ":" << i; cerr.flush();
        }
        int id = rand() % values.size();
        int k = values[id].back();
        cout.write((char *)&k, sizeof(k));
        values[id].pop_back();
      }
      cout << endl;
    }
  }
  cerr << endl;
  return 0;
}