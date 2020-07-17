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

    transport_ptr_->open();
    set_state_ok();
  } catch (const std::exception &e) {
    LOG_ERROR << "create thrift client exception: " << e.what()
              << ", connect param: " << conn_param.ToString();
    set_state_error();
  }
}

ThriftClient *ThriftClientFactory::Create(const ConnectionParam &conn_param) {
  return new ThriftClient(conn_param);
}

void ThriftClientFactory::Destroy(ThriftClient *p) { delete p; }

}  // namespace ww