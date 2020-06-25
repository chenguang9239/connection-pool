//
// Created by admin on 2019-01-15.
//

#ifndef CPPSERVER_ZKCHILDRENWATCHER_H
#define CPPSERVER_ZKCHILDRENWATCHER_H

#include <CppZooKeeper/CppZooKeeper.h>

#include <atomic>
#include <boost/algorithm/string/trim.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <functional>
#include <memory>

#include "Utils.h"
#include "ZKConfig.h"
#include "log.h"

namespace ww {

template <class T>
class ZKChildrenWatcher {
 public:
  void Init();

  ZKChildrenWatcher(const ZKConfig &config);

  bool GlobalWatcherFunc(CppZooKeeper::ZookeeperManager &zk_cli, int type,
                         int state, const char *path);

  bool WatcherFunc(CppZooKeeper::ZookeeperManager &zk_cli, int type, int state,
                   const char *path);

  void InitValueList();

  std::string GetNodeValue(const std::string &path);

  void set_reconnect_notifier(const std::function<void()> &func) {
    reconnect_notifier_ = func;
  }

  void set_resume_global_watcher_notifier(const std::function<void()> &func) {
    resume_global_watcher_notifier_ = func;
  }

  void set_resume_custom_watcher_notifier(const std::function<void()> &func) {
    resume_custom_watcher_notifier_ = func;
  }

  void set_resume_ephemeral_node_notifier(const std::function<void()> &func) {
    resume_ephemeral_node_notifier_ = func;
  }

  void InnerReconnectNotifier();

  void InnerResumeGlobalWatcherNotifier();

  void InnerResumeCustomWatcherNotifier();

  void InnerResumeEphemeralNodeNotifier();

 protected:
  void ZKChildrenHandler();

  T &Cast() { return static_cast<T &>(*this); }

  ZKConfig config_;
  CppZooKeeper::ZookeeperManager zk_client_;
  CppZooKeeper::ScopedStringVector string_vector_;

  bool need_init_value_list_;
  boost::shared_mutex node_value_list_smtx_;

  std::vector<std::string> node_value_list_;
  std::vector<std::string> additional_value_list_;
  std::vector<std::string> deleted_value_list_;

  std::function<void()> reconnect_notifier_;
  std::function<void()> resume_global_watcher_notifier_;
  std::function<void()> resume_custom_watcher_notifier_;
  std::function<void()> resume_ephemeral_node_notifier_;

  std::shared_ptr<CppZooKeeper::WatcherFuncType> global_watcher_ptr_;
};

template <class T>
bool ZKChildrenWatcher<T>::GlobalWatcherFunc(
    CppZooKeeper::ZookeeperManager &zk_cli, int type, int state,
    const char *path) {
  if (type == ZOO_SESSION_EVENT) {
    // 第一次连接成功与超时之后的重连成功，会触发ZOO_CONNECTED_STATE
    if (state == ZOO_CONNECTED_STATE) {
      // 不超时的重连成功也会触发会触发ZOO_CONNECTED_STATE
      if (need_init_value_list_) {
        LOG_SPCL << "first connection success, call InitValueList";
        need_init_value_list_ = false;
        InitValueList();
      }  // 超时会触发ZOO_EXPIRED_SESSION_STATE
    } else if (state == ZOO_EXPIRED_SESSION_STATE) {
      //      need_init_value_list_ = true;
    }
  }
  return false;
}

template <class T>
bool ZKChildrenWatcher<T>::WatcherFunc(CppZooKeeper::ZookeeperManager &zk_cli,
                                       int type, int state, const char *path) {
  if (type == ZOO_CHILD_EVENT || (type == CppZooKeeper::RESUME_EVENT &&
                                  state == CppZooKeeper::RESUME_SUCC)) {
    InitValueList();
  } else {
    LOG_ERROR << "not expected event, type: " << type << ", state: " << state
              << ", path: " << std::string(path ? path : "");
  }
  return false;
}

template <class T>
std::string ZKChildrenWatcher<T>::GetNodeValue(const std::string &path) {
  char *buffer = nullptr;
  Stat stat;
  int buf_len = 0;

  int ret = zk_client_.Get(path, nullptr, &buf_len, &stat);
  if (ret != ZOK || stat.dataLength <= 0) {
    //        LOG_ERROR << "ret: " << ret << ", dataLength: " <<
    //        stat.dataLength;
    return "";
  }

  buffer = (char *)malloc(sizeof(char) * (stat.dataLength + 1));
  buf_len = stat.dataLength + 1;

  ret = zk_client_.Get(path, buffer, &buf_len, &stat);
  if (ret != ZOK || buf_len <= 0) {
    //        LOG_ERROR << "ret: " << ret << ", buf_len: " << buf_len;
    if (buffer != nullptr) {
      free(buffer);
      buffer = nullptr;
    }
    return "";
  }

  std::string return_str;
  if (buffer) {
    return_str = std::string(buffer, buf_len);
  } else {
    LOG_ERROR << "returned path buffer is nullptr";
  }

  if (buffer != nullptr) {
    free(buffer);
    buffer = nullptr;
  }

  boost::algorithm::trim_if(return_str, boost::algorithm::is_any_of("/"));
  return return_str;
}

template <class T>
void ZKChildrenWatcher<T>::InitValueList() {
  zk_client_.GetChildren(
      config_.path, string_vector_,
      std::make_shared<CppZooKeeper::WatcherFuncType>(
          std::bind(&ZKChildrenWatcher::WatcherFunc, this,
                    std::placeholders::_1, std::placeholders::_2,
                    std::placeholders::_3, std::placeholders::_4)));

  std::vector<std::string> new_node_value_list;
  LOG_SPCL << "zkPath: " << config_.path
           << ", get children number: " << string_vector_.count;
  for (auto i = 0; i < string_vector_.count; ++i) {
    new_node_value_list.emplace_back(
        GetNodeValue(config_.path + '/' + string_vector_.data[i]));
    LOG_SPCL << "node path: " << string_vector_.data[i]
             << ", node data: " << new_node_value_list.back();
  }

  additional_value_list_ =
      Utils::VectorMinus(new_node_value_list, node_value_list_);
  deleted_value_list_ =
      Utils::VectorMinus(node_value_list_, new_node_value_list);

  for (const auto &e : additional_value_list_) {
    LOG_SPCL << "to be added value: " << e;
  }
  for (const auto &e : deleted_value_list_) {
    LOG_SPCL << "to be deleted value: " << e;
  }

  Cast().ZKChildrenHandler();

  if (!additional_value_list_.empty() || !deleted_value_list_.empty()) {
    boost::unique_lock<boost::shared_mutex> g(node_value_list_smtx_);
    // 先删除， 后追加
    auto tmp = node_value_list_;
    Utils::UpdateVector(node_value_list_, additional_value_list_, tmp,
                        deleted_value_list_);
  }

  LOG_SPCL << "zkPath: " << config_.path
           << ", updated children node value list size: "
           << node_value_list_.size();
}

template <class T>
ZKChildrenWatcher<T>::ZKChildrenWatcher(const ZKConfig &ZKConfig)
    : config_(ZKConfig) {
  need_init_value_list_ = true;
  global_watcher_ptr_ =
      std::make_shared<CppZooKeeper::WatcherFuncType>(std::bind(
          &ZKChildrenWatcher::GlobalWatcherFunc, this, std::placeholders::_1,
          std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  reconnect_notifier_ =
      std::bind(&ZKChildrenWatcher::InnerReconnectNotifier, this);
  resume_global_watcher_notifier_ =
      std::bind(&ZKChildrenWatcher::InnerResumeGlobalWatcherNotifier, this);
  resume_custom_watcher_notifier_ =
      std::bind(&ZKChildrenWatcher::InnerResumeCustomWatcherNotifier, this);
  resume_ephemeral_node_notifier_ =
      std::bind(&ZKChildrenWatcher::InnerResumeEphemeralNodeNotifier, this);
}

template <class T>
void ZKChildrenWatcher<T>::Init() {
  zk_client_.Init(config_.address);
  zk_client_.SetReconnectOptions(reconnect_notifier_,
                                 config_.userReconnectAlertCount);
  zk_client_.SetResumeOptions(
      resume_ephemeral_node_notifier_, resume_custom_watcher_notifier_,
      resume_global_watcher_notifier_, config_.userResumeAlertCount);
  zk_client_.SetCallWatcherFuncOnResume(true);
  int i = 0;
  while (++i <= 3) {
    if (ZOK == zk_client_.Connect(global_watcher_ptr_, config_.recvTimeout,
                                  config_.connTimeout)) {
      break;
    } else {
      LOG_ERROR << "connect to zk server error, time(s): " << i
                << ", zk addr: " << config_.address;
    }
  }
}

template <class T>
void ZKChildrenWatcher<T>::InnerReconnectNotifier() {
  LOG_WARN << "reconnection reaches " << config_.userReconnectAlertCount
           << " times, zk address: " << config_.address
           << ", zk path: " << config_.path;
}

template <class T>
void ZKChildrenWatcher<T>::InnerResumeGlobalWatcherNotifier() {
  LOG_WARN << "recovery of global watcher reaches "
           << config_.userResumeAlertCount
           << " times, zk address: " << config_.address
           << ", zk path: " << config_.path;
}

template <class T>
void ZKChildrenWatcher<T>::InnerResumeCustomWatcherNotifier() {
  LOG_WARN << "recovery of custom watcher reaches "
           << config_.userResumeAlertCount
           << " times, zk address: " << config_.address
           << ", zk path: " << config_.path;
}

template <class T>
void ZKChildrenWatcher<T>::InnerResumeEphemeralNodeNotifier() {
  LOG_WARN << "recovery of ephemeral node reaches "
           << config_.userResumeAlertCount
           << " times, zk address: " << config_.address
           << ", zk path: " << config_.path;
}

}  // namespace ww

#endif  // CPPSERVER_ZKCHILDRENWATCHER_H
