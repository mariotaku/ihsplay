pkg_check_modules(LIBCEC libcec)

if (LIBCEC_FOUND)
    set(IHSPLAY_FEATURE_LIBCEC ON PARENT_SCOPE)
    target_include_directories(ihsplay SYSTEM PRIVATE ${LIBCEC_INCLUDE_DIRS})
    target_link_libraries(ihsplay PRIVATE ${LIBCEC_LIBRARIES})

    target_sources(ihsplay PRIVATE cec_support.c cec_key.c)
else ()
    message("libcec not found, disabling support")
    set(IHSPLAY_FEATURE_LIBCEC OFF PARENT_SCOPE)
endif ()