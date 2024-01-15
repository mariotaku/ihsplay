get_filename_component(CMAKE_C_COMPILER_NAME "${CMAKE_C_COMPILER}" NAME)
if (CMAKE_C_COMPILER_NAME MATCHES "^arm-webos-linux-gnueabi-")
    set(CPACK_PACKAGE_ARCHITECTURE "arm")
else ()
    message(FATAL_ERROR "Unknown build architecture inferred from C compiler ${CMAKE_C_COMPILER_NAME}")
endif ()

set(CPACK_PACKAGE_NAME "org.mariotaku.ihsplay")
set(CPACK_GENERATOR "External")
set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/AresPackage.cmake")
set(CPACK_EXTERNAL_ENABLE_STAGING TRUE)
set(CPACK_MONOLITHIC_INSTALL TRUE)
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${PROJECT_VERSION}_${CPACK_PACKAGE_ARCHITECTURE}")
set(CPACK_PRE_BUILD_SCRIPTS "${CMAKE_SOURCE_DIR}/cmake/CleanupNameLink.cmake")

# Copy manifest
configure_file(deploy/webos/appinfo.in.json ./appinfo.json @ONLY)

install(TARGETS ihsplay RUNTIME DESTINATION .)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/deploy/webos/ DESTINATION . PATTERN ".*" EXCLUDE PATTERN "*.in" EXCLUDE
        PATTERN "*.in.json" EXCLUDE)
install(FILES "${CMAKE_BINARY_DIR}/appinfo.json" DESTINATION .)

# Will use all cores on CMake 3.20+
set(CPACK_THREADS 0)

if (NOT ENV{CI})
    add_custom_target(webos-package-ihsplay COMMAND cpack DEPENDS ihsplay)

    if (ENV{ARES_DEVICE})
        set(ares_arguments "-d" $ENV{ARES_DEVICE})
    endif ()

    add_custom_target(webos-install-ihsplay
            COMMAND ares-install "${CPACK_PACKAGE_FILE_NAME}.ipk" ${ares_arguments}
            WORKING_DIRECTORY ${CPACK_PACKAGE_DIRECTORY}
            DEPENDS webos-package-ihsplay
    )

    add_custom_target(webos-launch-ihsplay
            COMMAND ares-launch "${CPACK_PACKAGE_NAME}" ${ares_arguments}
            DEPENDS webos-install-ihsplay
    )

endif ()

include(CPack)