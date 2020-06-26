#ifndef CONNECTION_POOL_THRIFTCLIENT_H
#define CONNECTION_POOL_THRIFTCLIENT_H

#include <memory>

#include "Connection.h"
#include "gen-cpp/TestThriftConnPoolService.h"

namespace ww {

typedef apache::thrift::stdcxx::shared_ptr<
    ::ww::TestThriftConnPoolServiceClient>
    ClientPtr;

typedef apache::thrift::stdcxx::shared_ptr<
    apache::thrift::transport::TTransport>
    TransportPtr;

class ThriftClient : public Connection {
 public:
  ThriftClient(const ConnectionParam &conn_param);

  ClientPtr client_ptr_;

 private:
  TransportPtr transport_ptr_;
};

class ThriftClientFactory : public ConnectionFactory<ThriftClient> {
 public:
  virtual ThriftClient *Create(const ConnectionParam &conn_param);
  virtual void Destroy(ThriftClient *p);
};

}  // namespace ww

#endif  // CONNECTION_POOL_THRIFTCLIENT_H
