#ifndef CONNECTION_POOL_MULTICONNECTIONPOOL_H
#define CONNECTION_POOL_MULTICONNECTIONPOOL_H

#include <algorithm>
#include <boost/thread/shared_mutex.hpp>
#include <memory>
#include <vector>

#include "ConnectionPool.h"
#include "ZKChildrenWatcher.h"
#include "log.h"

namespace ww {

struct MultiConnectionPoolParam {
  ConnectionPoolParam conn_pool_param;
  ZKConfig zk_config;
  int retry;  // wait retry times to init pools
};

template <class C, class F>
class MultiConnectionPool
    : public ZKChildrenWatcher<MultiConnectionPool<C, F>> {
  using ZKType = ZKChildrenWatcher<MultiConnectionPool<C, F>>;

 public:
  MultiConnectionPool(const MultiConnectionPoolParam &multi_conn_pool_param);

  void ZKChildrenHandler();

  virtual std::shared_ptr<C> GetNextConn() = 0;

 protected:
  boost::shared_mutex pools_smtx_;
  std::vector<std::shared_ptr<ConnectionPool<C>>> pools_;
  ConnectionPoolParam conn_pool_param_;
  ConnectionPoolFactory<C, F> factory_;
};

template <class C, class F>
MultiConnectionPool<C, F>::MultiConnectionPool(
    const MultiConnectionPoolParam &multi_conn_pool_param)
    : ZKType(multi_conn_pool_param.zk_config),
      conn_pool_param_(multi_conn_pool_param.conn_pool_param) {
  if (conn_pool_param_.init_size < 1) {
    LOG_WARN << "invalid conn pool init size: " << conn_pool_param_.init_size
             << ", set default value: 1";
    conn_pool_param_.init_size = 1;
  }

  this->Init();

  for (int i = 0; i < multi_conn_pool_param.retry; ++i) {
    boost::shared_lock<boost::shared_mutex> g(pools_smtx_);
    if (pools_.empty()) {
      g.unlock();
      usleep(200000);
      continue;
    } else {
      LOG_WARN << "wait to init multi connection pool ...";
      break;
    }
  }

  {
    boost::shared_lock<boost::shared_mutex> g(pools_smtx_);
    if (pools_.empty()) {
      LOG_ERROR << "init multi connection pool failed";
    } else {
      LOG_SPCL << "init multi connection pool count: " << pools_.size();
    }
  }
}

// todo try catch

template <class C, class F>
void MultiConnectionPool<C, F>::ZKChildrenHandler() {
  // 先删除， 后追加
  if (!this->additional_value_list_.empty() ||
      !this->deleted_value_list_.empty()) {
    this->template UpdateAbstractNodes<ConnectionPoolFactory<C, F>,
                                       ConnectionPool<C>, ConnectionPoolParam>(
        pools_, pools_smtx_, this->node_value_list_,
        this->additional_value_list_, this->deleted_value_list_, factory_,
        conn_pool_param_);

    if (pools_.empty()) {
      LOG_WARN << "pools empty!";
    } else {
      LOG_SPCL
          << "handle changed zk nodes(in corresponding pools) ok, pool count: "
          << pools_.size();
    }
  } else {
    LOG_SPCL << "no zk node to be added or deleted";
  }
}

}  // namespace ww

#endif  // CONNECTION_POOL_MULTICONNECTIONPOOL_H
