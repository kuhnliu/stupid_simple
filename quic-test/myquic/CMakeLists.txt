include_directories(${CMAKE_SOURCE_DIR}/net)
set(myquic_FILES
my_quic_header.cc
my_quic_framer.cc
quic_framer_visitor.cc
my_quic_sender.cc
my_quic_receiver.cc
fakevideogenerator.cc
my_quic_video_sender.cc
my_quic_utils.cc
)
add_library(myquic STATIC ${myquic_FILES})
