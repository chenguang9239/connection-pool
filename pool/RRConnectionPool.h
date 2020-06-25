//
// Created by admin on 2020-06-16.
//

#ifndef CONNECTION_POOL_RRCONNECTIONPOOL_H
#define CONNECTION_POOL_RRCONNECTIONPOOL_H

#include "MultiConnectionPool.h"
#include "RoundRobin.h"

namespace ww {
template <class CONN, class FACTORY>
class RRConnectionPool : public RoundRobin, MultiConnectionPool<CONN, FACTORY> {
  using PoolType = MultiConnectionPool<CONN, FACTORY>;

 public:
  RRConnectionPool(const MultiConnectionPoolParam &MultiConnectionPoolParam);
  virtual std::shared_ptr<CONN> GetNextItem();

 private:
};

template <class CONN, class FACTORY>
RRConnectionPool<CONN, FACTORY>::RRConnectionPool(
    const MultiConnectionPoolParam &MultiConnectionPoolParam)
    : PoolType(MultiConnectionPoolParam) {}

template <class CONN, class FACTORY>
std::shared_ptr<CONN> RRConnectionPool<CONN, FACTORY>::GetNextItem() {
  boost::shared_lock<boost::shared_mutex> g(PoolType::pools_smtx_);
  if (PoolType::pools_.empty()) {
    return nullptr;
  }
  return PoolType::pools_[GetNext(PoolType::pools_.size())]->Borrow();
}

}  // namespace ww

#endif  // CONNECTION_POOL_RRCONNECTIONPOOL_H
