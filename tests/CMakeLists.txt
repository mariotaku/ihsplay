enable_testing()

function(ihsplay_add_test NAME)
    cmake_parse_arguments(TEST "" "" "SOURCES;ARGS;INCLUDES;LIBRARIES" ${ARGN})
    set(IHSPLAY_TEST_CASE "ihsplay_test_${NAME}")

    add_executable(${IHSPLAY_TEST_CASE} ${TEST_SOURCES})
    target_include_directories(${IHSPLAY_TEST_CASE} PRIVATE ${CMAKE_SOURCE_DIR}/app ${TEST_INCLUDES})
    if (TEST_LIBRARIES)
        target_link_libraries(${IHSPLAY_TEST_CASE} PRIVATE ${TEST_LIBRARIES})
    endif ()
    set_target_properties(${IHSPLAY_TEST_CASE} PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH_USE_LINK_PATH TRUE
            INSTALL_RPATH "${IHSPLAY_MODULE_LIBRARY_OUTPUT_DIRECTORY}")

    add_test(${NAME} ${IHSPLAY_TEST_CASE} ${TEST_ARGS})
    set_tests_properties(${NAME} PROPERTIES SKIP_RETURN_CODE 127)
    set(IHSPLAY_TEST_CASE ${IHSPLAY_TEST_CASE} PARENT_SCOPE)
endfunction()

add_subdirectory(app)