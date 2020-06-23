//
// Created by admin on 2020-06-16.
//

#ifndef CONNECTION_POOL_RRCONNECTIONPOOL_H
#define CONNECTION_POOL_RRCONNECTIONPOOL_H

#include "MultiConnectionPool.h"
#include "RoundRobin.h"

namespace ww {
template <class T>
class RRConnectionPool : public RoundRobinUtil, MultiConnectionPool<T> {
 public:
  RRConnectionPool(const MultiConnectionPoolParam &MultiConnectionPoolParam);
  virtual std::shared<T> GetNextItem();

 private:
};

template <class T>
RRConnectionPool<T>::RRConnectionPool(
    const MultiConnectionPoolParam &MultiConnectionPoolParam)
    : MultiConnectionPool<T>(MultiConnectionPoolParam) {}

template <class T>
std::shared<T> RRConnectionPool<T>::GetNextItem() {
  boost::shared_lock<boost::shared_mutex> g(pools_s_mtx);
  if (pools.empty()) {
    return nullptr;
  }
  return pools[GetNext(pools.size())].Borrow();
}

}  // namespace ww

#endif  // CONNECTION_POOL_RRCONNECTIONPOOL_H
