target_sources(ihsplay PRIVATE array_list.c listeners_list.c random.c version_info.c)

if (TARGET_WEBOS)
    pkg_check_modules(PBNJSON_C REQUIRED pbnjson_c)
    target_sources(ihsplay PRIVATE os_info_webos.c)
    target_include_directories(ihsplay PRIVATE ${PBNJSON_C_INCLUDE_DIRS})
    target_link_libraries(ihsplay PRIVATE ${PBNJSON_C_LIBRARIES} helpers)
else ()
    target_sources(ihsplay PRIVATE os_info_linux.c)
endif ()

add_subdirectory(video)