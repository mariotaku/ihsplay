add_library(ihsplay-mod-common INTERFACE)
target_include_directories(ihsplay-mod-common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ihsplay-mod-common PUBLIC INTERFACE ihslib-interface)