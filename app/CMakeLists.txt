add_executable(ihsplay
        main.c
        app.c
        app_events.c
        app_gamepad.c
        )

add_subdirectory(settings)
add_subdirectory(backend)
add_subdirectory(lvgl)
add_subdirectory(ui)
add_subdirectory(util)
add_subdirectory(platform)