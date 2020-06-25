//
// Created by admin on 2020-06-17.
//

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

  virtual std::shared_ptr<CONN> GetNextItem() = 0;

 protected:
  std::shared_ptr<ConnectionPool<CONN>> CreatePool(const std::string &ip_port);

  boost::shared_mutex pools_smtx_;
  std::vector<std::shared_ptr<ConnectionPool<CONN>>> pools_;
  ConnectionPoolParam conn_pool_param_;
};

// todo try catch

template <class CONN, class FACTORY>
std::shared_ptr<ConnectionPool<CONN>>
MultiConnectionPool<CONN, FACTORY>::CreatePool(const std::string &target) {
  auto ip_port = Utils::Split(target);
  auto conn_pool_param = conn_pool_param_;
  conn_pool_param.conn_param.ip = ip_port.at(0);
  conn_pool_param.conn_param.port = std::stod(ip_port.at(1));

  auto factory = std::make_shared<FACTORY>();
  return std::make_shared<ConnectionPool<CONN>>(factory, conn_pool_param);
}

// todo try catch

template <class CONN, class FACTORY>
MultiConnectionPool<CONN, FACTORY>::MultiConnectionPool(
    const MultiConnectionPoolParam &multi_conn_pool_param)
    : ZKType(multi_conn_pool_param.zk_config),
      conn_pool_param_(multi_conn_pool_param.conn_pool_param) {
  for (int i = 0; i < multi_conn_pool_param.retry; ++i) {
    boost::shared_lock<boost::shared_mutex> g(ZKType::node_value_list_smtx_);

    if (ZKType::node_value_list_.empty()) {
      g.unlock();
      usleep(200000);
      continue;
    } else {
      for (auto &value : ZKType::node_value_list_) {
        // todo 检查有效性
        pools_.emplace_back(CreatePool(value));
      }
    }
  }
}

// todo try catch

template <class CONN, class FACTORY>
void MultiConnectionPool<CONN, FACTORY>::ZKChildrenHandler() {
  std::vector<std::shared_ptr<ConnectionPool<CONN>>> additional_pools;
  for (auto &value : ZKType::additional_value_list_) {
    if (value.empty()) {
      LOG_ERROR << "zk node value empty! valueList size: "
                << ZKType::node_value_list_.size();
      continue;
    }

    // todo 检查有效性
    additional_pools.emplace_back(CreatePool(value));
    LOG_DEBUG << "in MultiConnectionPool ZKChildrenHandler";

    // 先删除， 后追加
    if (!ZKType::additional_value_list_.empty() ||
        !ZKType::deleted_value_list_.empty()) {
      boost::unique_lock<boost::shared_mutex> g(pools_smtx_);
      Utils::UpdateVector(pools_, additional_pools, ZKType::node_value_list_,
                          ZKType::deleted_value_list_, true);

      if (pools_.empty()) {
        LOG_ERROR << "no valid codis proxy!";
      } else {
        LOG_SPCL << "init redis pool ok, redis pool number: " << pools_.size();
      }
    } else {
      LOG_SPCL << "no node to be added or deleted";
    }
    LOG_SPCL << "codis redis client number: " << pools_.size();
  }
}

}  // namespace ww

#endif  // CONNECTION_POOL_MULTICONNECTIONPOOL_H
