add_library(ihslib-interface INTERFACE)
target_include_directories(ihslib-interface SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/third_party/ihslib/include)

add_subdirectory(common)

add_subdirectory(video)