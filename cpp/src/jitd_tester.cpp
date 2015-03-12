
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <string>
#include <stack>
#include <cstdlib>
#include <algorithm>
#include <sys/time.h>

#include "jitd.hpp"
#include "test.hpp"
#include "policy/cracker.hpp"

using namespace std;
using namespace std::placeholders;

RecordBuffer buffer_cmd(istream &toks)
{
  string fill;
  toks >> fill;

  if(string("random") == fill) {
    int len, max;
    toks >> len >> max;
    return build_buffer(len,max);        

  } else if(string("explicit") == fill) {
    return load_buffer(toks);
  
  } else if(string("file") == fill) {
    ifstream f;
    string filename;
    toks >> filename;
    f.open(filename);
    return load_buffer(f);        
    
  } else {
    cerr << "Invalid Fill Mode: " << fill << endl;
    exit(-1);
  }
}

double total_time(timeval &start, timeval &end)
{
  return (end.tv_sec - start.tv_sec) * 1000000.0 +
         (end.tv_usec - start.tv_usec); 
}

#define CASE_1(s) toks >> op; if(string(s) == op)
#define CASE(s) else if(string(s) == op)
  
void jitd_test(JITD<Record> &jitd, istream &input, bool interactive)
{
  string line;
  if(interactive) { cout << "jitd> "; cout.flush(); }
  while(getline(input, line)){
    istringstream toks(line);
    string op;
    
    ///////////////// MUTATOR OPERATIONS /////////////////
    CASE_1("insert") {
      jitd.insert(buffer_cmd(toks));
    } CASE("remove") {
      jitd.remove(buffer_cmd(toks));

    ///////////////// POLICY OPERATIONS /////////////////
    } CASE("policy") {
      string policyName;
      toks >> policyName;
      if(string("naive") == policyName){
        jitd.setPolicy(
          RewritePolicy<Record>(new RewritePolicyBase<Record>())
        );
      } else if(string("cracker") == policyName){
        int minSize;
        toks >> minSize;
        jitd.setPolicy(
          RewritePolicy<Record>(new CrackerPolicy<Record>(minSize))
        );
      }
      cout << "Now using policy: " << jitd.getPolicy()->name() << endl;

    ///////////////// ACCESS OPERATIONS /////////////////    
    } CASE("scan") {
      Iterator<Record> iter = jitd.iterator();
      timeval start, end;
      int idx = 0;
      
      while(!toks.eof() && !iter->atEnd()){
        CASE_1("flush") { // no argument
          int startidx = idx;
          gettimeofday(&start, NULL);
          while(!iter->atEnd()){ iter->next(); idx++; }
          gettimeofday(&end, NULL);
          cout << "Records Flushed: " << (idx - startidx) << endl;
          cout << "Flush Time: " << total_time(start, end) << " us" << endl;
        } CASE("full") { // no argument
          while(!iter->atEnd()){ 
            cout << idx << " -> " << iter->get()->key << endl;
            iter->next(); 
            idx++; 
          }
        } CASE("step") { // no argument
          cout << idx << " -> " << iter->get()->key << endl;
          idx++;
          iter->next();
        } CASE("next") { // [CNT]
          int cnt;
          for(toks >> cnt; (cnt > 0) && !iter->atEnd(); cnt--){
            cout << idx << " -> " << iter->get()->key << endl;
            idx++;
            iter->next();
          }
        } CASE("quiet_next") { // [CNT]
          int cnt, startidx = idx;
          for(toks >> cnt; (cnt > 0) && !iter->atEnd(); cnt--){ 
            idx++;
            iter->next();
          }
        } CASE("seek") { // [TARGET]
          Record target;
          toks >> target.key;
          target.value = NULL;
          gettimeofday(&start, NULL);
          iter->seek(target);
          gettimeofday(&end, NULL);
          cout << "Seek Time: " << total_time(start, end) << " us" << endl;
        } CASE("quiet_seek") { // [TARGET]
          Record target;
          toks >> target.key;
          target.value = NULL;
          iter->seek(target);
        }
      }
    } CASE("dump") {
      jitd.printDebug();
      
    ///////////////// OOOPS /////////////////
    } else {
      cerr << "Invalid Test Operation: " << op << endl;
      exit(-1);
    }
    if(interactive) { cout << "jitd> "; cout.flush(); }
  }
}