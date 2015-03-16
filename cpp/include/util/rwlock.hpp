#ifndef _RWLOCK_H_SHIELD
#define _RWLOCK_H_SHIELD

#include <atomic>
#include <mutex>

class RWLock
{
  std::atomic_uint readers, writers, read_waiters, write_waiters;
  std::mutex lock;
  std::condition_variable read, write;
  
  public:
    RWLock() : 
      readers(0), writers(0), read_waiters(0), write_waiters(0),
      lock(), read(), write() {}

    void reader_lock();
    void reader_release();
    void writer_lock();
    void writer_release();
  
};

#endif //_RWLOCK_H_SHIELD