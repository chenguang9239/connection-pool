cmake_minimum_required(VERSION 2.8.11)
project(test_thrift_server)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DBOOST_LOG_DYN_LINK -DTHREADED -O3 -g -msse4 -lpthread  -fopenmp")

set(SRC_ROOT "${CMAKE_SOURCE_DIR}")
set(DEPEND_INC "/usr/local/include")
set(DEPEND_LIB "/usr/local/lib")

include_directories(${SRC_ROOT}/../../thrift-client)
include_directories(${DEPEND_INC})


link_directories(${DEPEND_LIB}/thrift_0_12_0_lib)
link_directories(${DEPEND_LIB}/boost_1_68_0_lib)
link_directories(${DEPEND_LIB}/libevent_2_1_lib)
link_directories(${DEPEND_LIB}/zookeeper_lib)


file(GLOB SOURCE_FILES
		"${SRC_ROOT}/*.cpp"
		"${SRC_ROOT}/../../thrift-client/gen-cpp/*.cpp"
		)

add_executable(test_thrift_server ${SOURCE_FILES})

# static library
target_link_libraries(test_thrift_server
		libzookeeper_mt.a
		)

# dynamic library
target_link_libraries(test_thrift_server
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