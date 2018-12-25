include_directories(${CMAKE_SOURCE_DIR}/razor)
set(razor_FILES
cf_crc32.c
cf_stream.c
posix.c
sim_proto.c

)
add_library(razor STATIC ${razor_FILES})
