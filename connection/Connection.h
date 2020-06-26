#ifndef CONNECTION_POOL_CONNECTION_H
#define CONNECTION_POOL_CONNECTION_H

#include <memory>

namespace ww {

struct ConnectionParam {
  std::string ip;
  int port;
  int conn_timeout;
  int sock_timeout;

  std::string ToString() const {
    return "ip:" + ip + ",port:" + std::to_string(port) +
           ",conn_timeout:" + std::to_string(conn_timeout) +
           ",sock_timeout:" + std::to_string(sock_timeout);
  }
};

enum class ConnectionStat { OK, TIMEOUT, ERROR };

class Connection {
 public:
  Connection() { set_state_ok(); }

  virtual ~Connection() {}

  bool IsOK() { return state == ConnectionStat::OK; }

  bool IsTimeout() { return state == ConnectionStat::TIMEOUT; }

  bool IsError() { return state == ConnectionStat::ERROR; }

  void set_state_ok() { state = ConnectionStat::OK; }

  void set_state_timeout() { state = ConnectionStat::TIMEOUT; }

  void set_state_error() { state = ConnectionStat::ERROR; }

  ConnectionStat get_state() { return state; }

  // more base class member (virtual) functions

 private:
  ConnectionStat state;
};

template <class T>
class ConnectionFactory {
 public:
  virtual T* Create(const ConnectionParam& conn_param) = 0;
  // there may be some other operations to do in Create method of subclass

  virtual void Destroy(T* conn) = 0;
};

}  // namespace ww

#endif  // CONNECTION_POOL_CONNECTION_H
