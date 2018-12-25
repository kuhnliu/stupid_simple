PROJECT(project)
cmake_minimum_required(VERSION 2.6)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
##clang
add_compile_options("-stdlib=libc++" "-lc++abi" "-v")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_definitions(-DWEBRTC_WIN)
	add_definitions(-DNOMINMAX -DUNICODE)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions(-DWEBRTC_POSIX -DWEBRTC_LINUX)
endif()
add_definitions(-fno-rtti)
include_directories(/home/zsy/webrtc_2018_1/src/)
include_directories(/home/zsy/webrtc_2018_1/src/third_party/abseil-cpp/)
LINK_DIRECTORIES(/home/zsy/webrtc_2018_1/src/out/Default/obj/)
#add_definitions(-std=c++11)

include_directories(${CMAKE_SOURCE_DIR}/razor)
include_directories(${CMAKE_SOURCE_DIR}/)
add_subdirectory(razor)
add_subdirectory(test)

set(simulation_FILES
sender.cc
receiver.cc
fakevideosource.cc
)
add_library(simulation STATIC ${simulation_FILES})

set(EXECUTABLE_NAME "t_sender")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/t_sender.cc)
target_link_libraries(${EXECUTABLE_NAME}  simulation razor webrtc pthread)

set(EXECUTABLE_NAME "t_receiver")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/t_receiver.cc)
target_link_libraries(${EXECUTABLE_NAME}  simulation razor webrtc pthread)
#cmake .. -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++
