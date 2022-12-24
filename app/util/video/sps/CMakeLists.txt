add_library(sps_util STATIC sps_util_h264.c sps_util_h265.c common.c bitstream.c)
target_include_directories(sps_util PUBLIC include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(tests)