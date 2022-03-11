cmake_minimum_required(VERSION 3.16)

project(ihsplay VERSION 0.0.1 LANGUAGES C)

# Use `pkg-config` to link needed libraries.
find_package(PkgConfig REQUIRED)

# Use SDL2 for window creation and event handling.
pkg_check_modules(SDL2 REQUIRED sdl2)

add_subdirectory(third_party/ihslib)

add_subdirectory(third_party/lvgl EXCLUDE_FROM_ALL)

target_include_directories(lvgl SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(lvgl PRIVATE ${SDL2_LIBRARIES})
target_compile_definitions(lvgl PUBLIC LV_CONF_PATH=../../../app/ui/lv_conf.h)


set(CMAKE_C_STANDARD 11)

add_executable(ihsplay app/main.c app/app.c app/ui/app_ui.c)

target_link_libraries(ihsplay PRIVATE lvgl ihslib)

# Link SDL2
target_include_directories(ihsplay SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(ihsplay PRIVATE ${SDL2_LIBRARIES})
