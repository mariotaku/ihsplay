set(CPACK_PACKAGE_NAME "org.mariotaku.ihsplay")
set(CPACK_GENERATOR "External")
set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/AresPackage.cmake")
set(CPACK_EXTERNAL_ENABLE_STAGING TRUE)
set(CPACK_MONOLITHIC_INSTALL TRUE)
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${PROJECT_VERSION}_$ENV{ARCH}")

# Copy manifest
configure_file(deploy/webos/appinfo.json.in ./appinfo.json @ONLY)

install(TARGETS ihsplay RUNTIME DESTINATION .)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/deploy/webos/ DESTINATION . PATTERN ".*" EXCLUDE PATTERN "*.in" EXCLUDE)
install(FILES "${CMAKE_BINARY_DIR}/appinfo.json" DESTINATION .)

# Will use all cores on CMake 3.20+
set(CPACK_THREADS 0)

include(CPack)

add_custom_target(ihsplay-package COMMAND cpack DEPENDS ihsplay)

add_custom_target(ihsplay-install
        COMMAND ares-install ${CMAKE_SOURCE_DIR}/dist/org.mariotaku.ihsplay_${PROJECT_VERSION}_arm.ipk
        DEPENDS ihsplay-package
        )

add_custom_target(ihsplay-launch
        COMMAND ares-launch org.mariotaku.ihsplay
        DEPENDS ihsplay-install
        )