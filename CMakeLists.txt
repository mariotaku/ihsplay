cmake_minimum_required(VERSION 3.16)

execute_process(COMMAND git describe --tags --abbrev=0 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE IHSPLAY_VERSION ERROR_QUIET)
if (IHSPLAY_VERSION)
    string(STRIP "${IHSPLAY_VERSION}" IHSPLAY_VERSION)
    string(SUBSTRING "${IHSPLAY_VERSION}" 1 -1 IHSPLAY_VERSION)
else ()
    set(IHSPLAY_VERSION "0.0.0")
endif ()

project(ihsplay VERSION ${IHSPLAY_VERSION} LANGUAGES C)

# To suppress warnings for MbedTLS
if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif ()
# To suppress warnings for ExternalProject DOWNLOAD_EXTRACT_TIMESTAMP
if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif ()

enable_testing()

# Somehow the directory containing FindMbedTLS.cmake must be in the first place
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/core/scripts ${CMAKE_SOURCE_DIR}/third_party/commons/cmake
        ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/cmake/sanitizers/cmake)

option(IHSPLAY_WIP_FEATURES "Enable Work-in-Progress Features" OFF)
option(IHSPLAY_FEATURE_FORCE_FULLSCREEN "Force full screen mode" OFF)

set(IHSPLAY_FEATURE_LIBCEC ON)

option(COMMONS_LV_SDL_IMG_USE_IMAGE "Don't use SDL_Image" OFF)

set(COMMONS_LOGGING_SDL ON)
set(COMMONS_LOGGING_LVGL ON)
set(COMMONS_LOGGING_SS4S ON)

get_filename_component(CMAKE_C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME)

if (CMAKE_C_COMPILER_NAME MATCHES "^arm-webos-linux-gnueabi-")
    set(TARGET_WEBOS TRUE)
endif ()

# Use `pkg-config` to link needed libraries.
find_package(PkgConfig REQUIRED)
find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(MbedTLS)

# Use SDL2 for window creation and event handling.
if (TARGET_WEBOS)
    set(CMAKE_INSTALL_LIBDIR lib/backports)
    set(SDL2_BACKPORT_REVISION "webOS-2.30.x")
    include(ExternalSDL2BackportForWebOS)
    unset(CMAKE_INSTALL_LIBDIR)
else ()
    pkg_check_modules(SDL2 REQUIRED sdl2>=2.0.14)
endif()
pkg_check_modules(PROTOBUF_C libprotobuf-c)
pkg_check_modules(OPUS opus)

if (NOT PROTOBUF_C_FOUND)
    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    message(STATUS "Including protobuf-c from source...")
    include(ExternalProtobufC)
    list(POP_BACK CMAKE_MESSAGE_INDENT)
endif ()

if (NOT OPUS_FOUND)
    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    message(STATUS "Including opus from source...")
    include(ExternalOPUS)
    list(POP_BACK CMAKE_MESSAGE_INDENT)
endif ()

if (NOT MBEDTLS_FOUND)
    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    message(STATUS "Including mbedtls from source...")
    include(ExternalMbedTLS)
    list(POP_BACK CMAKE_MESSAGE_INDENT)
endif ()

if (TARGET_WEBOS)
#    set(IHSLIB_SDL_TARGETVERSION "2.28.0")
    set(SS4S_MODULE_DISABLE_MMAL ON)
endif ()

add_subdirectory(core)

add_subdirectory(third_party/lvgl EXCLUDE_FROM_ALL)

target_include_directories(lvgl SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(lvgl SYSTEM PRIVATE ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(lvgl PRIVATE ${SDL2_LIBRARIES} ${FREETYPE_LIBRARIES})
target_compile_definitions(lvgl PUBLIC LV_CONF_PATH=../../../app/lvgl/lv_conf.h)

add_subdirectory(third_party/ss4s)
add_subdirectory(third_party/commons)

set(CMAKE_C_STANDARD 11)

add_subdirectory(app)
target_include_directories(ihsplay PRIVATE app)

target_link_libraries(ihsplay PRIVATE lvgl ihslib ihslib-hid-sdl ss4s commons-array-list commons-os-info
        commons-logging commons-ss4s-modules-list)
target_link_libraries(ihsplay PRIVATE lv_gridview)

get_target_property(SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY ss4s SS4S_MODULE_LIBRARY_OUTPUT_DIRECTORY)

target_include_directories(ihsplay SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS} ${OPUS_INCLUDE_DIRS} ${MBEDTLS_INCLUDE_DIRS})
target_link_libraries(ihsplay PRIVATE ${SDL2_LIBRARIES} ${OPUS_LIBRARIES} Fontconfig::Fontconfig Freetype::Freetype
        ${MBEDCRYPTO_LIBRARY})

if (TARGET_WEBOS)
    set(IHSPLAY_FEATURE_LIBCEC OFF)
    set(IHSPLAY_FEATURE_FORCE_FULLSCREEN ON)
    target_link_libraries(ihsplay PRIVATE commons-luna-sync)

    set_target_properties(ihsplay PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH_USE_LINK_PATH TRUE
            INSTALL_RPATH "$ORIGIN/lib/backports:$ORIGIN")
    include(PackageWebOS)
else ()
    set(CMAKE_INSTALL_PREFIX /usr)
    include(GNUInstallDirs)
endif ()

if (IHSPLAY_FEATURE_LIBCEC AND TARGET commons-cec-sdl)
    target_link_libraries(ihsplay PRIVATE commons-cec-sdl)
else ()
    set(IHSPLAY_FEATURE_LIBCEC OFF)
endif ()

if (IHSPLAY_SANITIZE_ADDRESS)
    target_compile_options(ihsplay PRIVATE -fsanitize=address)
    target_link_options(ihsplay PRIVATE -fsanitize=address)
    target_link_libraries(ihsplay PRIVATE asan)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(IHSPLAY_IS_DEBUG ON)
endif ()

configure_file(app/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)
target_include_directories(ihsplay PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(tests)