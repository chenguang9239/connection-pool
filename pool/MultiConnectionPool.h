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
      conn_pool_ptr->Init();
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
    boost::shared_lock<boost::shared_mutex> g(this->node_value_list_smtx_);

    if (this->node_value_list_.empty()) {
      g.unlock();
      usleep(200000);
      continue;
    } else {
      for (auto &value : this->node_value_list_) {
        auto pool = CreatePool(value);
        if (pool) {
          pools_.emplace_back(std::move(pool));
          LOG_INFO << "init pool ok: " << value;
        } else {
          LOG_ERROR << "init pool error: " << value;
        }
      }
    }
  }
}

// todo try catch

template <class CONN, class FACTORY>
void MultiConnectionPool<CONN, FACTORY>::ZKChildrenHandler() {
  // 先删除， 后追加
  if (!this->additional_value_list_.empty() ||
      !this->deleted_value_list_.empty()) {
    std::function<std::shared_ptr<ConnectionPool<CONN>>(const std::string &)>
        builder = std::bind(&MultiConnectionPool<CONN, FACTORY>::CreatePool,
                            this, std::placeholders::_1);

    this->UpdateVector(pools_, pools_smtx_, this->node_value_list_,
                       this->additional_value_list_, this->deleted_value_list_,
                       builder);

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
