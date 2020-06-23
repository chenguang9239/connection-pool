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
  int retry;
};

template <class T>
class MultiConnectionPool : public ZKChildrenWatcher<MultiConnectionPool> {
 public:
  MultiConnectionPool(const MultiConnectionPoolParam &multi_conn_pool_param);
  void ZKChildrenHandler();
  virtual std::shared<T> GetNextItem() = 0;

 protected:
  std::shared_ptr<ConnectionPool<T>> CreatePool(const std::string &ip_port);

  boost::shared_mutex pools_s_mtx;
  std::vector<std::shared_ptr<ConnectionPool<T>>> pools;
  ConnectionPoolParam conn_pool_param_;
};

// todo try catch

template <class T>
std::shared_ptr<ConnectionPool<T>> MultiConnectionPool<T>::CreatePool(
    const std::string &target) {
  auto ip_port = Utils::Split(target);
  auto conn_pool_param = conn_pool_param_;
  conn_pool_param.ip = ip_port.at(0);
  conn_pool_param.port = std::stod(ip_port.at(1));

  return std::make_shared<ConnectionPool<T>>(conn_pool_param);
}

// todo try catch

template <class T>
void MultiConnectionPool<T>::MultiConnectionPool(
    const MultiConnectionPoolParam &multi_conn_pool_param)
    : conn_pool_param_(multi_conn_pool_param.conn_pool_param) {
  for (int i = 0; i < multi_conn_pool_param.retry; ++i) {
    boost::shared_lock<boost::shared_mutex> g(node_value_list_smtx);

    if (node_value_list_.empty()) {
      g.unlock();
      usleep(200000);
      continue;
    } else {
      for (auto &value : node_value_list_) {
        // todo 检查有效性
        pools.emplace_back(CreatePool<T>(value));
      }
    }
  }
}

// todo try catch

template <class T>
void MultiConnectionPool<T>::ZKChildrenHandler() {
  std::vector<std::shared_ptr<ConnectionPool<T>>> additional_pools;
  for (auto &value : additional_value_list_) {
    if (value.empty()) {
      LOG_ERROR << "zk node value empty! valueList size: " << valueList.size();
      continue;
    }

    // todo 检查有效性
    additional_pools.emplace_back(CreatePool<T>(value));
    LOG_DEBUG << "in MultiConnectionPool ZKChildrenHandler";

    // 先删除， 后追加
    if (!additional_value_list_.empty() || !deleted_value_list_.empty()) {
      boost::unique_lock<boost::shared_mutex> g(poolListSMtx);
      Utils::UpdateVector(pools, additional_pools, deleted_value_list_, true);

      if (pools.empty()) {
        LOG_ERROR << "no valid codis proxy!";
      } else {
        LOG_SPCL << "init redis pool ok, redis pool number: " << pools.size();
      }
    } else {
      LOG_SPCL << "no node to be added or deleted";
    }
    LOG_SPCL << "codis redis client number: " << pools.size();
  }
}

}  // namespace ww

#endif  // CONNECTION_POOL_MULTICONNECTIONPOOL_H
