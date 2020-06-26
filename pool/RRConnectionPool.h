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
  virtual std::shared_ptr<CONN> GetNextConn();

 private:
};

template <class CONN, class FACTORY>
RRConnectionPool<CONN, FACTORY>::RRConnectionPool(
    const MultiConnectionPoolParam &MultiConnectionPoolParam)
    : PoolType(MultiConnectionPoolParam) {}

template <class CONN, class FACTORY>
std::shared_ptr<CONN> RRConnectionPool<CONN, FACTORY>::GetNextConn() {
  boost::shared_lock<boost::shared_mutex> g(this->pools_smtx_);
  if (this->pools_.empty()) {
    return nullptr;
  }
  return this->pools_[GetNext(this->pools_.size())]->Borrow();
}

}  // namespace ww

#endif  // CONNECTION_POOL_RRCONNECTIONPOOL_H
