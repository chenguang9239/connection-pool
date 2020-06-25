cmake_minimum_required(VERSION 2.8.11)
project(pool)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DBOOST_LOG_DYN_LINK -DTHREADED -O3 -g -msse4 -lpthread  -fopenmp")

set(SRC_ROOT "${CMAKE_SOURCE_DIR}")
set(DEPEND_INC "/usr/local/include")
set(DEPEND_LIB "/usr/local/lib")


include_directories(${SRC_ROOT}/connection)
include_directories(${SRC_ROOT}/pool)
include_directories(${SRC_ROOT}/thrift-client)
include_directories(${SRC_ROOT}/utils)
include_directories(${SRC_ROOT}/zk-children-watcher)
include_directories(${SRC_ROOT}/log)
include_directories(${SRC_ROOT}/utils)
include_directories(${DEPEND_INC})


link_directories(${DEPEND_LIB}/thrift_0_12_0_lib)
link_directories(${DEPEND_LIB}/boost_1_68_0_lib)
link_directories(${DEPEND_LIB}/libevent_2_1_lib)
link_directories(${DEPEND_LIB}/zookeeper_lib)
link_directories(${DEPEND_LIB}/cpp_zookeeper_api_lib)


file(GLOB SOURCE_FILES
		"${SRC_ROOT}/*.cpp"
		"${SRC_ROOT}/thrift-client/gen-cpp/*.cpp"
		"${SRC_ROOT}/thrift-client/*.cpp"
		"${SRC_ROOT}/utils/*.cpp"
		"${SRC_ROOT}/zk-children-watcher/*.cpp"
		"${SRC_ROOT}/test/*.cpp"
		)

#add_custom_target(compile_thrift
#      COMMAND cd ${SRC_ROOT}/thrift-client/proto && thrift -r --gen cpp test.thrift && mv -f gen-cpp/* ../gen-cpp/
#      COMMENT "compile prediction.thrift "
#     )

# add_dependencies(user_profile_operator compile_prediction_thrift)

#add_library(POOL_CLIENT STATIC ${SOURCE_FILES})
#
#set(POOL_CLIENT "./pool-client")
#
#install(TARGETS POOL_CLIENT DESTINATION ${POOL_CLIENT}/lib)
#
#install(DIRECTORY ./
#		DESTINATION ${POOL_CLIENT}/include
#		FILES_MATCHING PATTERN "*.h"
#		PATTERN "build" EXCLUDE
#		PATTERN "thrift" EXCLUDE
#		PATTERN "demo-build" EXCLUDE)

add_executable(pool ${SOURCE_FILES})

# static library
target_link_libraries(pool
		libCppZooKeeper.a
		libzookeeper_mt.a
		)

# dynamic library
target_link_libraries(pool
		event
		thrift
		thriftnb
		boost_log_setup
		boost_log
		boost_thread
		boost_date_time
		boost_serialization
		boost_regex
		boost_system
		boost_chrono
		boost_iostreams
		pthread
		)