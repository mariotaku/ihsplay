install(TARGETS ihsplay RUNTIME DESTINATION .)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/deploy/webos/ DESTINATION .)

set(CPACK_PACKAGE_NAME "org.mariotaku.ihsplay")
set(CPACK_GENERATOR "External")
set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/AresPackage.cmake")
set(CPACK_EXTERNAL_ENABLE_STAGING TRUE)
set(CPACK_MONOLITHIC_INSTALL TRUE)
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${PROJECT_VERSION}_$ENV{ARCH}")

# Will use all cores on CMake 3.20+
set(CPACK_THREADS 0)

include(CPack)

add_custom_target(ihsplay-package COMMAND cpack DEPENDS ihsplay)

add_custom_target(ihsplay-install
        COMMAND echo ${WEBOS_PACKAGE_PATH}
        COMMAND ares-install ${CMAKE_SOURCE_DIR}/dist/org.mariotaku.ihsplay_0.0.1_arm.ipk
        DEPENDS ihsplay-package
        )