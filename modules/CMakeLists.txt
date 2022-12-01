add_library(ihslib-interface INTERFACE)
target_include_directories(ihslib-interface SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/core/include)

add_subdirectory(common)

add_subdirectory(video)