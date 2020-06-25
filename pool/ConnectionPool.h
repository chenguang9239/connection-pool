// Define your custom logging function by overriding this #define
#ifndef _DEBUG
#define _DEBUG(x)
#endif

#include <deque>
#include <exception>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "Connection.h"
#include "log.h"

namespace ww {

struct ConnectionPoolStats {
  size_t pool_size;
  size_t borrowed_size;

  ConnectionPoolStats(size_t pool_size, size_t borrowed_size)
      : pool_size(pool_size), borrowed_size(borrowed_size) {}
};

struct ConnectionPoolParam {
  ConnectionParam conn_param;
  int init_size;
  int max_size;
  int retry;
};

template <class T>
class ConnectionPool {
 public:
  ConnectionPoolStats get_stats() {
    std::lock_guard<std::mutex> lock(mtx_);
    return ConnectionPoolStats(pool_.size(), borrowed_.size());
  };

  ConnectionPool(const std::shared_ptr<ConnectionFactory<T>> factory,
                 const ConnectionPoolParam& conn_pool_param)
      : factory_(factory), conn_pool_param_(conn_pool_param) {
    while (pool_.size() < conn_pool_param_.init_size) {
      // 检查有效性
      pool_.emplace_back(factory_->Create(conn_pool_param_.conn_param));
    }
  };

  ~ConnectionPool(){};

  std::shared_ptr<T> Borrow() {
    std::lock_guard<std::mutex> lock(mtx_);

    std::shared_ptr<T> conn = nullptr;

    if (!pool_.empty()) {
      conn = pool_.front();
      pool_.pop_front();
      borrowed_.insert(conn);
    } else if (borrowed_.size() < conn_pool_param_.max_size) {
      conn = factory_->Create(conn_pool_param_.conn_param);
      // 检查有效性
      borrowed_.insert(conn);
    } else {
      LOG_ERROR << "no available connection!";
    }

    return conn;
  };

  void Relase(std::shared_ptr<T> conn) {
    std::lock_guard<std::mutex> lock(mtx_);
    pool_.push_back(conn);
    borrowed_.erase(conn);
  };

 protected:
  std::shared_ptr<ConnectionFactory<T>> factory_;
  ConnectionPoolParam conn_pool_param_;
  std::deque<std::shared_ptr<T> > pool_;
  std::set<std::shared_ptr<T> > borrowed_;
  std::mutex mtx_;
};

}  // namespace ww