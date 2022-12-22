add_executable(ihsplay
        main.c
        app.c
        app_events.c
        app_gamepad.c
        app_logging.c
        )

add_subdirectory(settings)
add_subdirectory(backend)
add_subdirectory(lvgl)
add_subdirectory(ui)
add_subdirectory(util)