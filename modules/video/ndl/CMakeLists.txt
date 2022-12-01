add_library(ihsplay-mod-ndl ndl_module.c)
target_include_directories(ihsplay-mod-ndl SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ihsplay-mod-ndl PRIVATE ihsplay-mod-common NDL_directmedia)