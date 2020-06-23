//
// Created by admin on 2020-06-18.
//

#include "ThriftClient.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>

#include "log.h"

namespace ww {

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

ThriftClient::ThriftClient(const ConnectionParam &conn_param) {
  try {
    apache::thrift::stdcxx::shared_ptr<TSocket> t_socket(
        new TSocket(conn_param.ip, conn_param.port));
    apache::thrift::stdcxx::shared_ptr<TTransport> socket(t_socket);
    transport_ptr_.reset(new TFramedTransport(socket));
    apache::thrift::stdcxx::shared_ptr<TProtocol> protocol(
        new TCompactProtocol(transport_ptr_));

    client_ptr_.reset(new TestThriftConnPoolServiceClient(protocol));

    t_socket->setConnTimeout(conn_param.conn_timeout);
    t_socket->setRecvTimeout(conn_param.sock_timeout);

  } catch (const std::exception &e) {
    LOG_ERROR << "create thrift client exception: " << e.what()
              << ", connect param: " << conn_param.ToString();
  }
}

std::shared_ptr<Connection> ThriftClientFactory::Create(
    const ConnectionParam &conn_param) {
  return std::make_shared<ThriftClient>(conn_param);
}

}  // namespace ww