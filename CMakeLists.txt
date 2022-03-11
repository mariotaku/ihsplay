cmake_minimum_required(VERSION 3.16)

project(ihsplay VERSION 0.0.1 LANGUAGES C)

# Use `pkg-config` to link needed libraries.
find_package(PkgConfig REQUIRED)

# Use SDL2 for window creation and event handling.
pkg_check_modules(SDL2 REQUIRED sdl2)

set(CMAKE_C_STANDARD 11)

add_executable(ihsplay main.c)
