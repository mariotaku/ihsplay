list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(BroadcomMMAL REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(OPUS opus REQUIRED)
pkg_check_modules(ALSA alsa REQUIRED)

add_library(ihsplay-mod-raspi raspi_module.c alsaaud.c mmalvid.c sps_parser.c)
target_include_directories(ihsplay-mod-raspi SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ihsplay-mod-raspi PRIVATE ihsplay-mod-common)

target_include_directories(ihsplay-mod-raspi PRIVATE ${CMAKE_SOURCE_DIR}/samples SYSTEM PRIVATE
        ${BROADCOM_INCLUDE_DIRS} ${OPUS_INCLUDE_DIRS} ${ALSA_INCLUDE_DIRS})
target_link_libraries(ihsplay-mod-raspi PRIVATE Threads::Threads ${BROADCOM_MMAL_LIBRARIES}
        ${OPUS_LIBRARIES} ${ALSA_LIBRARIES})
