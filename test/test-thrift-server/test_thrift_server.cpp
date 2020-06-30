#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TNonblockingServerSocket.h>

#include "gen-cpp/TestThriftConnPoolService.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace apache::thrift::concurrency;
using ::apache::thrift::stdcxx::shared_ptr;

using namespace ::ww;

class TestThriftConnPoolServiceHandler
    : virtual public TestThriftConnPoolServiceIf {
 public:
  TestThriftConnPoolServiceHandler() {
    // Your initialization goes here
  }

  int64_t Predict(const int64_t input) {
    // Your implementation goes here
    printf("Predict\n");
    return input * input;
  }
};

::apache::thrift::stdcxx::shared_ptr<TNonblockingServer> server_ptr;

int main(int argc, char **argv) {
  int port = 9090;

  shared_ptr<TestThriftConnPoolServiceHandler> handler(
      new TestThriftConnPoolServiceHandler());
  shared_ptr<TProcessor> processor(
      new TestThriftConnPoolServiceProcessor(handler));
  shared_ptr<TNonblockingServerSocket> socket_ptr(
      new TNonblockingServerSocket(port));
  shared_ptr<TNonblockingServerTransport> serverTransport(socket_ptr.get());
  shared_ptr<TProtocolFactory> protocolFactory(new TCompactProtocolFactory());
  shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory());
  shared_ptr<ThreadManager> threadManager =
      ThreadManager::newSimpleThreadManager(20);
  threadManager->threadFactory(threadFactory);
  threadManager->start();
  server_ptr.reset(new TNonblockingServer(processor, protocolFactory,
                                          serverTransport, threadManager));

  TOverloadAction overload_action = T_OVERLOAD_DRAIN_TASK_QUEUE;
  server_ptr->setOverloadAction(overload_action);
  server_ptr->setNumIOThreads(5);
  //        server_ptr->setUseHighPriorityIOThreads(true);
  server_ptr->setResizeBufferEveryN(0);
  server_ptr->setIdleReadBufferLimit(0);
  server_ptr->setIdleWriteBufferLimit(0);
  server_ptr->setMaxFrameSize(1024 * 1024);
  server_ptr->setWriteBufferDefaultSize(1024 * 1024);

  server_ptr->serve();
  return 0;
}