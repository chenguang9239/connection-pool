#ifndef CONNECTION_POOL_ROUNDROBIN_H
#define CONNECTION_POOL_ROUNDROBIN_H

#include <atomic>

class RoundRobin {
  std::atomic<uint64_t> size;
  std::atomic<uint64_t> index;

 public:
  RoundRobin();
  RoundRobin(uint64_t new_size);
  void SetSize(uint64_t new_size);
  uint64_t GetSize();
  uint64_t GetNext();
  uint64_t GetNext(uint64_t n);
};

#endif  // CONNECTION_POOL_ROUNDROBIN_H
