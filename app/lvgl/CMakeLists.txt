target_sources(ihsplay PRIVATE
        display.c
        mouse.c
        keypad.c
        theme.c
        widgets/gridview/lv_gridview.c)

add_subdirectory(widgets)