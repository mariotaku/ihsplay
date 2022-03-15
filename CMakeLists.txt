cmake_minimum_required(VERSION 3.16)

project(ihsplay VERSION 0.0.1 LANGUAGES C)

option(ENABLE_SANITIZER "Enable memory sanitizer (only for debug builds)" OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if (ENABLE_SANITIZER)
    string(APPEND CMAKE_C_FLAGS_DEBUG " -fno-omit-frame-pointer -fsanitize=address")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fno-omit-frame-pointer -fsanitize=address")
    string(APPEND CMAKE_LINKER_FLAGS_DEBUG " -fno-omit-frame-pointer -fsanitize=address")
    message("-- Sanitizer enabled")
endif ()

# Use `pkg-config` to link needed libraries.
find_package(PkgConfig REQUIRED)

# Use SDL2 for window creation and event handling.
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(PROTOBUF_C libprotobuf-c)

find_library(MBEDCRYPTO_LIBRARY mbedcrypto)

if ("$ENV{CROSS_COMPILE}" STREQUAL "arm-webos-linux-gnueabi-")
    set(TARGET_WEBOS TRUE)
endif ()

if (NOT PROTOBUF_C_FOUND)
    option(BUILD_PROTOC "Disable protoc-c" OFF)
    add_subdirectory(third_party/protobuf-c/build-cmake EXCLUDE_FROM_ALL)
    set(PROTOBUF_C_FOUND 1)
    set(PROTOBUF_C_LIBRARIES protobuf-c)
    set(PROTOBUF_C_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/third_party/protobuf-c)
endif ()

if (NOT MBEDCRYPTO_LIBRARY)
    option(ENABLE_TESTING "Disable MbedTLS testing" OFF)
    option(ENABLE_PROGRAMS "Disable MbedTLS programs" OFF)
    add_subdirectory(third_party/mbedtls EXCLUDE_FROM_ALL)
    set(MBEDTLS_FOUND 1)
    set(MBEDCRYPTO_LIBRARY mbedcrypto)
endif ()

add_subdirectory(third_party/ihslib)

add_subdirectory(third_party/lvgl EXCLUDE_FROM_ALL)

target_include_directories(lvgl SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(lvgl PRIVATE ${SDL2_LIBRARIES})
target_compile_definitions(lvgl PUBLIC LV_CONF_PATH=../../../app/lvgl/lv_conf.h)


set(CMAKE_C_STANDARD 11)

add_executable(ihsplay
        app/main.c
        app/app.c
        app/backend/hosts_manager.c
        app/lvgl/display.c
        app/lvgl/mouse.c
        app/lvgl/theme.c
        app/lvgl/lv_gridview.c
        app/ui/app_ui.c
        app/ui/launcher.c
        app/ui/session.c
        app/ui/hosts/hosts_fragment.c
        app/util/array_list.c
        )
target_include_directories(ihsplay PRIVATE app)

target_link_libraries(ihsplay PRIVATE lvgl ihslib)

# Link SDL2
target_include_directories(ihsplay SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(ihsplay PRIVATE ${SDL2_LIBRARIES})

if (TARGET_WEBOS)
    add_subdirectory(system/webos/tv-native-apis)
endif ()

add_subdirectory(modules)
target_link_libraries(ihsplay PRIVATE ihsplay-mod-common)

if (TARGET_WEBOS)
    target_link_libraries(ihsplay PRIVATE ihsplay-mod-ndl)
    include(PackageWebOS)
else ()
    target_link_libraries(ihsplay PRIVATE ihsplay-mod-raspi)
endif ()