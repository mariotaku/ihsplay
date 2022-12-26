cmake_minimum_required(VERSION 3.16)

project(ihsplay VERSION 0.1.0 LANGUAGES C)

enable_testing()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/cmake/sanitizers/cmake)

# Use `pkg-config` to link needed libraries.
find_package(PkgConfig REQUIRED)
find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)

# Use SDL2 for window creation and event handling.
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(PROTOBUF_C libprotobuf-c)
pkg_check_modules(OPUS opus)

find_library(MBEDCRYPTO_LIBRARY mbedcrypto)

option(IHSPLAY_WIP_FEATURES "Enable Work-in-Progress Features" OFF)
set(IHSPLAY_FEATURE_RELMOUSE ON)

if ("$ENV{CROSS_COMPILE}" STREQUAL "arm-webos-linux-gnueabi-")
    set(TARGET_WEBOS TRUE)
    # TODO: Remove usage of ENV{ARCH}
    set(ENV{ARCH} "arm")
endif ()

if (NOT PROTOBUF_C_LIBRARIES OR NOT PROTOBUF_C_INCLUDEDIR)
    message("Using protobuf-c submodule")
    option(BUILD_PROTOC "Disable protoc-c" OFF)
    option(BUILD_TESTS "Disable protoc-c tests" OFF)
    add_subdirectory(third_party/protobuf-c/build-cmake EXCLUDE_FROM_ALL)
    set(PROTOBUF_C_FOUND 1)
    set(PROTOBUF_C_LIBRARIES protobuf-c)
    set(PROTOBUF_C_INCLUDEDIR ${CMAKE_SOURCE_DIR}/third_party/protobuf-c)
endif ()

if (NOT OPUS_FOUND)
    message("Using opus submodule")
    set(BUILD_TESTING OFF)
    add_subdirectory(third_party/opus EXCLUDE_FROM_ALL)
    unset(BUILD_TESTING)
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

if (TARGET_WEBOS)
    set(IHSLIB_SDL_TARGETVERSION "2.0.0")
    set(SS4S_MODULE_DISABLE_MMAL ON)
endif ()

add_subdirectory(core)

add_subdirectory(third_party/lvgl EXCLUDE_FROM_ALL)

include(BuildINIH)

target_include_directories(lvgl SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(lvgl SYSTEM PRIVATE ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(lvgl PRIVATE ${SDL2_LIBRARIES} ${FREETYPE_LIBRARIES})
target_compile_definitions(lvgl PUBLIC LV_CONF_PATH=../../../app/lvgl/lv_conf.h)


if (TARGET_WEBOS)
    add_subdirectory(system/webos/tv-native-apis)
endif ()

add_subdirectory(third_party/ss4s)

set(CMAKE_C_STANDARD 11)

add_subdirectory(app)
target_include_directories(ihsplay PRIVATE app)

target_link_libraries(ihsplay PRIVATE lvgl ihslib ihslib-hid-sdl ss4s inih)
target_link_libraries(ihsplay PRIVATE lv_gridview)

get_target_property(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY ss4s SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY)

set_target_properties(ihsplay PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
        INSTALL_RPATH "${SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY}")

target_include_directories(ihsplay SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS} ${OPUS_INCLUDE_DIRS} ${MBEDCRYPTO_INCLUDE_DIRS})
target_link_libraries(ihsplay PRIVATE ${SDL2_LIBRARIES} ${OPUS_LIBRARIES} Fontconfig::Fontconfig Freetype::Freetype
        ${MBEDCRYPTO_LIBRARY})

if (TARGET_WEBOS)
    set(SS4S_MODULES_INI_PATH_RUNTIME "${CMAKE_INSTALL_LIBDIR}/ss4s_modules.ini")
    set(IHSPLAY_FEATURE_RELMOUSE OFF)
    target_link_libraries(ihsplay PRIVATE rt)
    include(PackageWebOS)
else ()
    # TODO: remove for packaged builds
    set(SS4S_MODULES_INI_PATH_RUNTIME ${SS4S_MODULES_INI_OUTPUT_FILE})
    set(CMAKE_INSTALL_PREFIX /usr)
    include(GNUInstallDirs)
endif ()

if (IHSPLAY_SANITIZE_ADDRESS)
    target_compile_options(ihsplay PRIVATE -fsanitize=address)
    target_link_options(ihsplay PRIVATE -fsanitize=address -static-libasan)
    target_link_libraries(ihsplay PRIVATE asan)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(IHSPLAY_IS_DEBUG ON)
endif ()

configure_file(app/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)
target_include_directories(ihsplay PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(tests)