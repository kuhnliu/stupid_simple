include_directories(${CMAKE_SOURCE_DIR}/net)
set(bbr_FILES
my_bbr_sender.cc
my_pacing_sender.cc
my_bbr_video_sender_app.cc
my_bbr_sender_app.cc
my_bbr_receiver_app.cc
)
add_library(bbr STATIC ${bbr_FILES})
