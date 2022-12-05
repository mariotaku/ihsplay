cmake_minimum_required(VERSION 3.16)

project(ihsplay VERSION 0.0.1 LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/cmake/sanitizers/cmake)

# Use `pkg-config` to link needed libraries.
find_package(PkgConfig REQUIRED)
find_package(Freetype REQUIRED)

find_package(Sanitizers)

# Use SDL2 for window creation and event handling.
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(PROTOBUF_C libprotobuf-c)
pkg_check_modules(OPUS opus)

find_library(MBEDCRYPTO_LIBRARY mbedcrypto)

if ("$ENV{CROSS_COMPILE}" STREQUAL "arm-webos-linux-gnueabi-")
    set(TARGET_WEBOS TRUE)
endif ()

if (NOT PROTOBUF_C_LIBRARIES OR NOT PROTOBUF_C_INCLUDEDIR)
    option(BUILD_PROTOC "Disable protoc-c" OFF)
    message("Using protobuf-c submodule")
    add_subdirectory(third_party/protobuf-c/build-cmake EXCLUDE_FROM_ALL)
    set(PROTOBUF_C_FOUND 1)
    set(PROTOBUF_C_LIBRARIES protobuf-c)
    set(PROTOBUF_C_INCLUDEDIR ${CMAKE_SOURCE_DIR}/third_party/protobuf-c)
endif ()

if (NOT OPUS_FOUND)
    message("Using opus submodule")
    add_subdirectory(third_party/opus EXCLUDE_FROM_ALL)
    set(OPUS_FOUND 1)
    set(OPUS_LIBRARIES opus)
    set(OPUS_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/third_party/opus/include)
endif ()

if (NOT MBEDCRYPTO_LIBRARY)
    option(ENABLE_TESTING "Disable MbedTLS testing" OFF)
    option(ENABLE_PROGRAMS "Disable MbedTLS programs" OFF)
    add_subdirectory(third_party/mbedtls EXCLUDE_FROM_ALL)
    set(MBEDTLS_FOUND 1)
    set(MBEDCRYPTO_LIBRARY mbedcrypto)
endif ()

add_subdirectory(core)
add_sanitizers(ihslib)

add_subdirectory(third_party/lvgl EXCLUDE_FROM_ALL)

target_include_directories(lvgl SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(lvgl SYSTEM PRIVATE ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(lvgl PRIVATE ${SDL2_LIBRARIES})
target_compile_definitions(lvgl PUBLIC LV_CONF_PATH=../../../app/lvgl/lv_conf.h)


if (TARGET_WEBOS)
    add_subdirectory(system/webos/tv-native-apis)
endif ()
add_subdirectory(third_party/ss4s)

set(CMAKE_C_STANDARD 11)

add_subdirectory(app)
target_include_directories(ihsplay PRIVATE app)

target_link_libraries(ihsplay PRIVATE lvgl ihslib ihslib-hid-sdl ss4s)
target_compile_definitions(ihslib-hid-sdl PRIVATE IHS_SDL_TARGETVERSION=2000)

get_target_property(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY ss4s SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY)
get_target_property(SS4S_MODULE_NAMES ss4s SS4S_MODULE_NAMES)

if (SS4S_MODULE_NAMES)
    foreach (SS4S_MODULE_NAME ${SS4S_MODULE_NAMES})
        install(TARGETS ${SS4S_MODULE_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endforeach ()
endif ()

set_target_properties(ihsplay PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
        INSTALL_RPATH "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}")

target_include_directories(ihsplay SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ${OPUS_INCLUDE_DIRS})
target_link_libraries(ihsplay PRIVATE ${SDL2_LIBRARIES} ${FREETYPE_LIBRARIES} ${OPUS_LIBRARIES})

if (TARGET_WEBOS)
    target_link_libraries(ihsplay PRIVATE rt)
    include(PackageWebOS)
endif ()

add_sanitizers(ihsplay)