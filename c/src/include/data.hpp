
#ifndef _DATA_HPP_SHIELD
#define _DATA_HPP_SHIELD

#include <memory>
#include <vector>

typedef long int Key;
typedef void *Value;

struct Record {
  Key   key;
  Value value;
  
  bool comp(Record &a, Record &b) { return a.key < b.key; }
  bool equiv(Record &a, Record &b) { return a.key == b.key; }
};

typedef std::shared_ptr< std::array<Record> > Buffer;

compare


#endif // _DATA_HPP_SHIELD
