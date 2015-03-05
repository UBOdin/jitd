
#ifndef _DATA_HPP_SHIELD
#define _DATA_HPP_SHIELD

#include <memory>
#include <vector>

typedef long int Key;
typedef void *Value;

struct Record {
  Key   key;
  Value value;
};

typedef std::shared_ptr< std::vector<Record> > Buffer;
typedef std::vector<Record>::const_iterator BufferElement;

struct CompareRecord {
  bool operator()(const Record &r1, const Record &r2){
    return r1.key < r2.key;
  }
};


#endif // _DATA_HPP_SHIELD
