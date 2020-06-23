//
// Created by admin on 2020-06-16.
//

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

class Connection {
 public:
  Connection(){};
  virtual ~Connection(){};
  void MoreBaseClassMemberFunctions(){};

  bool state;
};

class ConnectionFactory {
 public:
  virtual std::shared_ptr<Connection> Create(
      const ConnectionParam& conn_param) = 0;
};

}  // namespace ww

#endif  // CONNECTION_POOL_CONNECTION_H
