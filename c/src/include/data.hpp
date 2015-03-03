
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


#endif // _DATA_HPP_SHIELD
