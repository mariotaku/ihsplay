pkg_check_modules(PBNJSON_C REQUIRED pbnjson_c)
target_sources(ihsplay PRIVATE app_webos.c lunasynccall.c client_info_webos.c os_info_webos.c)
target_include_directories(ihsplay PRIVATE ${PBNJSON_C_INCLUDE_DIRS})
target_link_libraries(ihsplay PRIVATE ${PBNJSON_C_LIBRARIES} helpers)