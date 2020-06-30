#include <unistd.h>

#include <iostream>
#include <thread>

#include "MultiConnectionPool.h"
#include "RRConnectionPool.h"
#include "ThriftClient.h"

int main() {
  ww::MultiConnectionPoolParam multi_conn_pool_param;

  multi_conn_pool_param.zk_config.address =
      "xxx:2181";
  multi_conn_pool_param.zk_config.path =
      "/xxx";
  multi_conn_pool_param.zk_config.recvTimeout = 500;
  multi_conn_pool_param.zk_config.connTimeout = 500;
  multi_conn_pool_param.zk_config.clientLogPath =
      "/xxx";
  multi_conn_pool_param.zk_config.clientLogLevel = 2;
  multi_conn_pool_param.zk_config.logPath =
      "/xxx";
  multi_conn_pool_param.zk_config.logLevel = 1;

  multi_conn_pool_param.retry = 3;

  multi_conn_pool_param.conn_pool_param.init_size = 5;
  multi_conn_pool_param.conn_pool_param.max_size = 30;
  multi_conn_pool_param.conn_pool_param.retry = 4;

  multi_conn_pool_param.conn_pool_param.conn_param.conn_timeout = 200;
  multi_conn_pool_param.conn_pool_param.conn_param.sock_timeout = 100;

  ww::RRConnectionPool<ww::ThriftClient, ww::ThriftClientFactory> rr_conn_pool(
      multi_conn_pool_param);

  if (rr_conn_pool.GetNextConn()) {
    std::cout << std::endl << "get connection ok" << std::endl;
  }

  auto func = [&](int i) {
    std::cout << "Predict result: "
              << rr_conn_pool.GetNextConn()->client_ptr_->Predict(i)
              << std::endl;
  };

  for (int i = 0; i < 200; ++i) {
    std::thread t(func, i);
    t.detach();
  }

  sleep(10);

  return 0;
}