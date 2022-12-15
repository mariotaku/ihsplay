target_sources(ihsplay PRIVATE
        app_ui.c
        app_ui_font.c
        launcher.c
        hosts/hosts_fragment.c
        settings/settings.c
        settings/basic.c
        settings/widgets.c)

add_subdirectory(common)
add_subdirectory(session)