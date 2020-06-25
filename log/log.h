//
// Created by admin on 2020-06-23.
//

#ifndef CONNECTION_POOL_LOG_H
#define CONNECTION_POOL_LOG_H

#include <iostream>

#ifndef LOG_DEBUG
#define LOG_DEBUG std::cout << std::endl
#endif

#ifndef LOG_INFO
#define LOG_INFO std::cout << std::endl
#endif

#ifndef LOG_WARN
#define LOG_WARN std::cout << std::endl
#endif

#ifndef LOG_ERROR
#define LOG_ERROR std::cout << std::endl
#endif

#ifndef LOG_SPCL
#define LOG_SPCL std::cout << std::endl
#endif

#endif  // CONNECTION_POOL_LOG_H
