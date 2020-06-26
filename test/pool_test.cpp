#include <iostream>

#include "MultiConnectionPool.h"
#include "RRConnectionPool.h"
#include "ThriftClient.h"

int main() {
  ww::MultiConnectionPoolParam multi_conn_pool_param;

  multi_conn_pool_param.retry = 3;

  multi_conn_pool_param.conn_pool_param.init_size = 5;
  multi_conn_pool_param.conn_pool_param.max_size = 30;
  multi_conn_pool_param.conn_pool_param.retry = 4;

  multi_conn_pool_param.conn_pool_param.conn_param.conn_timeout = 200;
  multi_conn_pool_param.conn_pool_param.conn_param.sock_timeout = 100;

  ww::RRConnectionPool<ww::ThriftClient, ww::ThriftClientFactory> rr_conn_pool(
      multi_conn_pool_param);

  rr_conn_pool.GetNextConn();

  std::cout << "Predict result: "
            << rr_conn_pool.GetNextConn()->client_ptr_->Predict(3) << std::endl;

  return 0;
}