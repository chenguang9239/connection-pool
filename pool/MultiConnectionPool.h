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
  int retry;
};

template <class CONN, class FACTORY>
class MultiConnectionPool
    : public ZKChildrenWatcher<MultiConnectionPool<CONN, FACTORY>> {
  using ZKType = ZKChildrenWatcher<MultiConnectionPool<CONN, FACTORY>>;

 public:
  MultiConnectionPool(const MultiConnectionPoolParam &multi_conn_pool_param);

  void ZKChildrenHandler();

  virtual std::shared_ptr<CONN> GetNextConn() = 0;

 protected:
  std::shared_ptr<ConnectionPool<CONN>> CreatePool(const std::string &target);

  boost::shared_mutex pools_smtx_;
  std::vector<std::shared_ptr<ConnectionPool<CONN>>> pools_;
  ConnectionPoolParam conn_pool_param_;
};

template <class CONN, class FACTORY>
std::shared_ptr<ConnectionPool<CONN>>
MultiConnectionPool<CONN, FACTORY>::CreatePool(const std::string &target) {
  std::shared_ptr<ConnectionPool<CONN>> conn_pool_ptr = nullptr;
  try {
    auto ip_port = this->Split(target);
    auto conn_pool_param = conn_pool_param_;
    conn_pool_param.conn_param.ip = ip_port.at(0);
    conn_pool_param.conn_param.port = std::stod(ip_port.at(1));

    auto factory = std::make_shared<FACTORY>();
    conn_pool_ptr =
        std::make_shared<ConnectionPool<CONN>>(factory, conn_pool_param);
    if (conn_pool_ptr) {
      LOG_SPCL << "create connection pool ok";
    } else {
      LOG_ERROR << "create connection pool error, nullptr!";
    }
  } catch (const std::exception &e) {
    LOG_ERROR << "create connection pool exception: " << e.what();
  }

  return conn_pool_ptr;
}

template <class CONN, class FACTORY>
MultiConnectionPool<CONN, FACTORY>::MultiConnectionPool(
    const MultiConnectionPoolParam &multi_conn_pool_param)
    : ZKType(multi_conn_pool_param.zk_config),
      conn_pool_param_(multi_conn_pool_param.conn_pool_param) {
  this->Init();

  for (int i = 0; i < multi_conn_pool_param.retry; ++i) {
    boost::shared_lock<boost::shared_mutex> g(pools_smtx_);
    if (pools_.empty()) {
      g.unlock();
      usleep(200000);
      continue;
    } else {
      LOG_WARN << "wait to init multi connection pool ...";
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

template <class CONN, class FACTORY>
void MultiConnectionPool<CONN, FACTORY>::ZKChildrenHandler() {
  static std::shared_ptr<ConnectionPool<CONN>> invalid_pool = nullptr;

  // 先删除， 后追加
  if (!this->additional_value_list_.empty() ||
      !this->deleted_value_list_.empty()) {
    std::function<std::shared_ptr<ConnectionPool<CONN>>(const std::string &)>
        builder = std::bind(&MultiConnectionPool<CONN, FACTORY>::CreatePool,
                            this, std::placeholders::_1);

    this->UpdateVector(pools_, pools_smtx_, this->node_value_list_,
                       this->additional_value_list_, this->deleted_value_list_,
                       builder, invalid_pool);

    if (pools_.empty()) {
      LOG_WARN << "pools empty!";
    } else {
      LOG_SPCL
          << "handle changed zk nodes(corresponding pools) ok, pool count: "
          << pools_.size();
    }
  } else {
    LOG_SPCL << "no zk node to be added or deleted";
  }
}

}  // namespace ww

#endif  // CONNECTION_POOL_MULTICONNECTIONPOOL_H
