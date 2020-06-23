//
// Created by admin on 2020-06-17.
//

#ifndef CONNECTION_POOL_ROUNDROBIN_H
#define CONNECTION_POOL_ROUNDROBIN_H

#include <atomic>

template <class T>
class RoundRobin {
  std::atomic<uint64_t> size;
  std::atomic<uint64_t> index;
  T& cast() { return static_cast<T&>(*this); }

 public:
  RoundRobinUtil();
  RoundRobinUtil(uint64_t new_size);
  void SetSize(uint64_t new_size);
  uint64_t GetSize();
  uint64_t GetNext();
  uint64_t GetNextItem(uint64_t n);
};

#endif //CONNECTION_POOL_ROUNDROBIN_H
