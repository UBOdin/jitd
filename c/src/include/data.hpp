
#ifndef _DATA_HPP_SHIELD
#define _DATA_HPP_SHIELD

#include <atomic>

typedef long int Key;
typedef void *Value;

struct Record {
  Key   key;
  Value value;
}

typedef shared_ptr<vector<Record>> Buffer;


#endif // _DATA_HPP_SHIELD
