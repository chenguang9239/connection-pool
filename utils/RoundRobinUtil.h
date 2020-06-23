//
// Created by admin on 2020-06-17.
//

#ifndef CONNECTION_POOL_ROUNDROBINUTIL_H
#define CONNECTION_POOL_ROUNDROBINUTIL_H

#include <atomic>

class RoundRobinUtil {
  std::atomic<uint64_t> size;
  std::atomic<uint64_t> index;

public:
  RoundRobinUtil();
  RoundRobinUtil(uint64_t new_size);
  void SetSize(uint64_t new_size);
  uint64_t GetSize();
  uint64_t GetNext();
  uint64_t GetNext(uint64_t n);
};

#endif // CONNECTION_POOL_ROUNDROBINUTIL_H
