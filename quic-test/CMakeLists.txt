PROJECT(project)
cmake_minimum_required(VERSION 2.6)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_C_FLAGS} -std=gnu++11")
include_directories(.)
include_directories(/home/zsy/posix_quic-master/libquic/)
LINK_DIRECTORIES(/home/zsy/posix_quic-master/build/libquic/)
LINK_DIRECTORIES(/home/zsy/posix_quic-master/build/libquic/third_party/boringssl/src/ssl)
LINK_DIRECTORIES(/home/zsy/posix_quic-master/build/libquic/third_party/boringssl/src/crypto)

set(frame_FILES
my_quic_framer.cc
quic_framer_visitor.cc
)
add_library(framer STATIC ${frame_FILES})
set(EXECUTABLE_NAME "t_recv")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/t_recv_packet_manager.cc)
target_link_libraries(${EXECUTABLE_NAME} framer quic crypto ssl pthread)
