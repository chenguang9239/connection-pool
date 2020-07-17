#include <atomic>
#include <deque>
#include <exception>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "AbstractNode.h"
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
  int retry;  // times of repeating to create connection failed
};

template <class T>
class ConnectionPool : public std::enable_shared_from_this<ConnectionPool<T>>,
                       public AbstractNode {
 public:
  ConnectionPool(const std::shared_ptr<ConnectionFactory<T>> factory,
                 const ConnectionPoolParam &conn_pool_param);

  ~ConnectionPool();

  std::shared_ptr<T> Borrow();

  void Release(T *conn);

  ConnectionPoolStats get_stats();

  virtual bool IsValid();

  void Init();

 protected:
  std::shared_ptr<ConnectionFactory<T>> factory_;
  ConnectionPoolParam conn_pool_param_;
  std::deque<T *> pool_;
  std::mutex mtx_;

 private:
  friend class Deleter;

  class Deleter {
   public:
    Deleter(std::shared_ptr<ConnectionPool<T>> pool) : pool_(pool) {}

    void operator()(T *conn) { pool_->Release(conn); }

   private:
    std::shared_ptr<ConnectionPool<T>> pool_;
  };

  std::atomic<int> borrowed_count_;
};

template <class T>
ConnectionPool<T>::ConnectionPool(
    const std::shared_ptr<ConnectionFactory<T>> factory,
    const ConnectionPoolParam &conn_pool_param)
    : factory_(factory), conn_pool_param_(conn_pool_param) {
  borrowed_count_ = 0;

  if (conn_pool_param_.init_size < 1) {
    LOG_WARN << "invalid conn pool init size: " << conn_pool_param_.init_size
             << ", set default value: 1";
    conn_pool_param_.init_size = 1;
  }

  for (int i = 0; i < conn_pool_param_.init_size; ++i) {
    for (int j = 0; j < conn_pool_param_.retry; ++j) {
      auto p = factory_->Create(conn_pool_param_.conn_param);
      if (p && p->IsOK()) {
        pool_.emplace_back(p);
        break;
      } else {
        LOG_ERROR << "init connection error: "
                  << conn_pool_param_.conn_param.ToString();
      }
    }
  }

  LOG_INFO << "init " << pool_.size()
           << " connections: " << conn_pool_param_.conn_param.ToString();
};

template <class T>
ConnectionPool<T>::~ConnectionPool() {
  for (auto p : pool_) {
    factory_->Destroy(p);
  }
  LOG_SPCL << "destroy " << pool_.size()
           << " connections: " << conn_pool_param_.conn_param.ToString();
};

template <class T>
std::shared_ptr<T> ConnectionPool<T>::Borrow() {
  T *conn = nullptr;

  {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!pool_.empty()) {
      conn = pool_.front();
      pool_.pop_front();
      ++borrowed_count_;
      return std::shared_ptr<T>(conn, Deleter(this->shared_from_this()));
    }
  }

  if (borrowed_count_ < conn_pool_param_.max_size) {
    conn = factory_->Create(conn_pool_param_.conn_param);
    LOG_INFO << "create connection, idle count: " << pool_.size()
             << ", borrowed count: " << borrowed_count_;
    if (conn) {
      ++borrowed_count_;
    }
  } else {
    LOG_ERROR << "no available connection!";
  }

  return std::shared_ptr<T>(conn, Deleter(this->shared_from_this()));
};

template <class T>
void ConnectionPool<T>::Release(T *conn) {
  if (conn) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (borrowed_count_ + pool_.size() > conn_pool_param_.max_size) {
      factory_->Destroy(conn);
      LOG_WARN << "destroy extra connection, idle count: " << pool_.size()
               << ", borrowed count: " << borrowed_count_;
    } else {
      pool_.push_back(conn);
    }
  } else {
    LOG_ERROR << "connection is nullptr!";
  }
  --borrowed_count_;

#ifdef DEBUG
  LOG_SPCL << "release, borrowd count: " << borrowed_count_;
#endif
};

template <class T>
ConnectionPoolStats ConnectionPool<T>::get_stats() {
  std::lock_guard<std::mutex> lock(mtx_);
  return ConnectionPoolStats(pool_.size(), borrowed_count_);
};

template <class T>
bool ConnectionPool<T>::IsValid() {
  std::lock_guard<std::mutex> lock(mtx_);
  return !pool_.empty();
}

template <class C, class F>
class ConnectionPoolFactory
    : public AbstractNodeFactory<ConnectionPoolFactory<C, F>, ConnectionPool<C>,
                                 ConnectionPoolParam> {
 public:
  std::shared_ptr<ConnectionPool<C>> Create(const ConnectionPoolParam &param);
};

template <class C, class F>
std::shared_ptr<ConnectionPool<C>> ConnectionPoolFactory<C, F>::Create(
    const ConnectionPoolParam &conn_pool_param) {
  std::shared_ptr<ConnectionPool<C>> conn_pool_ptr = nullptr;
  try {
    auto factory = std::make_shared<F>();
    conn_pool_ptr =
        std::make_shared<ConnectionPool<C>>(factory, conn_pool_param);
    if (conn_pool_ptr && conn_pool_ptr->get_stats().pool_size > 0) {
      LOG_SPCL << "create connection pool ok";
    } else {
      LOG_ERROR
          << "create connection pool error, nullptr or empty! conn count: "
          << conn_pool_ptr->get_stats().pool_size;
      conn_pool_ptr = nullptr;
    }
  } catch (const std::exception &e) {
    LOG_ERROR << "create connection pool exception: " << e.what();
    conn_pool_ptr = nullptr;
  }

  return conn_pool_ptr;
}

}  // namespace ww