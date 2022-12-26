execute_process(COMMAND ares-package "${CPACK_TEMPORARY_DIRECTORY}" -o "${CPACK_PACKAGE_DIRECTORY}")

find_program(GEN_MANIFEST_CMD webosbrew-gen-manifest)
if (GEN_MANIFEST_CMD)
    execute_process(COMMAND "${GEN_MANIFEST_CMD}"
            -a "${CPACK_TEMPORARY_DIRECTORY}/appinfo.json"
            -p "${CPACK_PACKAGE_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}.ipk"
            -o "${CPACK_PACKAGE_DIRECTORY}/${CPACK_PACKAGE_NAME}.manifest.json"
            -i "https://github.com/mariotaku/ihsplay/raw/master/deploy/webos/largeIcon.png"
            -l "https://github.com/mariotaku/ihsplay"
            )
else ()
    message("Skip webOS homebrew manifest generation because command line tool is not found")
endif ()