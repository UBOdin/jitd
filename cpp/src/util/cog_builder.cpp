
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <stack>
#include <cstdlib>
#include <algorithm>

#include "cog.hpp"
#include "cog_builder.hpp"

using namespace std;

Buffer build_buffer(int len)
{
  int i;
  Buffer buff(new vector<Record>(len));
  for(i = 0; i < len; i++){
    (*buff)[i].key = rand() % 1000000;
    (*buff)[i].value = (Value)0xDEADBEEF;
  }
  return buff;
}

Buffer load_buffer(istream &input)
{
  vector<Record> temp;
  Record r;
  r.value = (Value)0xDEADBEEF;
  while(!input.eof()){
    input >> r.key;
    temp.push_back(r);
  }
  return Buffer(new vector<Record>(temp));
}

CogHandle array_for_buffer(Buffer buff)
{
  return MakeHandle(new ArrayCog(buff, buff->begin(), buff->end()));
}


CogHandle build_random_array(int len)
{
  return array_for_buffer(build_buffer(len));
}

CogHandle build_random_sorted_array(int len)
{
  Buffer buff = build_buffer(len);
  sort(buff->begin(), buff->end(), CompareRecord());
  return MakeHandle(new SortedArrayCog(buff, buff->begin(), buff->end()));
}

CogHandle build_cog(istream &input)
{
  shared_ptr<string> curr;
  stack<CogHandle> ret;
  string line;
  
  while(getline(input, line)){
    istringstream toks(line);
    string type;
    
    toks >> type;
    
    if(string("array") == type){
      string fill;
      toks >> fill;
      
      if(string("random") == fill) {
        int len;
        toks >> len;
        ret.push(build_random_array(len));        
      } else if(string("explicit") == fill) {
        ret.push(array_for_buffer(load_buffer(toks)));
      } else {
        cerr << "Invalid ArrayCog Fill Mode: " << fill << endl;
        exit(-1);
      }
      
    } else if(string("concat") == type) {
      CogHandle a = ret.top(); ret.pop();
      CogHandle b = ret.top(); ret.pop();
      
      ret.push(MakeHandle(new ConcatCog(a, b)));
    } else if(string("btree") == type) {
      Key sep;
      toks >> sep;
      
      CogHandle b = ret.top(); ret.pop();
      CogHandle a = ret.top(); ret.pop();

      ret.push(MakeHandle(new BTreeCog(a, sep, b)));
    } else {
      cerr << "Invalid Cog Type: " << type << endl;
      exit(-1);
    }
  }
  
  CogHandle handle = ret.top();
  ret.pop();
  return handle;
}