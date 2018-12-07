include_directories(${CMAKE_SOURCE_DIR}/test)
set(test_FILES
test_sender.cc
)
add_library(test STATIC ${test_FILES})

set(EXECUTABLE_NAME "t_video")
add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/test/t_video.cc)
target_link_libraries(${EXECUTABLE_NAME} razor test simulation webrtc pthread)
