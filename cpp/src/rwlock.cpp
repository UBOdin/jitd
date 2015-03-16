#include <iostream>

#include "util/rwlock.hpp"

using namespace std;

void RWLock::reader_lock()
{
  unique_lock<std::mutex> l(lock);
  if(writers.load() || write_waiters.load())
  {
    read_waiters.fetch_add(1);
    do { read.wait(l); }
    while(writers.load() || write_waiters.load());
    read_waiters.fetch_sub(1);
  }
  readers.fetch_add(1);
}

void RWLock::reader_release()
{
  unique_lock<std::mutex> l(lock);
  readers.fetch_sub(1);
  if(writers.load() > 0){
    write.notify_all();
  }
}

void RWLock::writer_lock()
{
  unique_lock<std::mutex> l(lock);
  if(readers.load() || writers.load()){
    write_waiters.fetch_add(1);
    do { write.wait(l); }
    while(readers.load() || writers.load());
    write_waiters.fetch_sub(1);
  }
  writers.store(1);
}

void RWLock::writer_release()
{
  unique_lock<std::mutex> l(lock);
  writers.store(0);
  if(write_waiters.load()){
    write.notify_all();
  } else if(read_waiters.load()){
    read.notify_all();
  }
}