
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

inline std::pair<Buffer,Buffer> 
  splitBuffer(BufferElement curr, BufferElement end, Key pivot)
{
  Buffer low(new std::vector<Record>());
  Buffer high(new std::vector<Record>());
  for(; curr < end; ++curr){
    if(curr->key < pivot){ low->push_back(*curr); }
    else                  { high->push_back(*curr); }
  }
  return std::pair<Buffer,Buffer>(low, high);
}
inline std::pair<Buffer,Buffer> 
  splitBuffer(Buffer b, Key pivot)
{
  return splitBuffer(b->begin(), b->end(), pivot);
}


#endif // _DATA_HPP_SHIELD
