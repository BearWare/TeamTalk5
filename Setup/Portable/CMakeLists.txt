set(PORTABLE_VERSION "v5.21.2.5188")
set(PORTABLE_OUTPUT_DIR "${TEAMTALK_ROOT}/Setup/Portable")

if(WIN32)
    find_program(7Z_EXECUTABLE 7z PATHS "C:/Program Files/7-Zip" "C:/Program Files (x86)/7-Zip" "$ENV{ProgramFiles}/7-Zip")
    if(NOT 7Z_EXECUTABLE)
        message(WARNING "7-Zip not found. Portable archive creation will fail.")
    endif()
endif()

function(create_windows_portable TARGET_NAME ARCH)
    set(DIST_NAME "TeamTalk_${PORTABLE_VERSION}_Portable-${ARCH}")
    set(DIST_DIR "${PORTABLE_OUTPUT_DIR}/${DIST_NAME}")

    if(ARCH STREQUAL "win32")
        set(EXTRA_DLLS "${CMAKE_INSTALL_PREFIX}/Client/qtTeamTalk/dolapi32.dll" "${CMAKE_INSTALL_PREFIX}/Client/qtTeamTalk/nvdaControllerClient32.dll" "${CMAKE_INSTALL_PREFIX}/Client/qtTeamTalk/SAAPI32.dll" "${CMAKE_INSTALL_PREFIX}/Client/qtTeamTalk/Tolk.dll")
    else()
        set(EXTRA_DLLS "${CMAKE_INSTALL_PREFIX}/Client/qtTeamTalk/nvdaControllerClient64.dll" "${CMAKE_INSTALL_PREFIX}/Client/qtTeamTalk/SAAPI64.dll" "${CMAKE_INSTALL_PREFIX}/Client/qtTeamTalk/Tolk.dll")
    endif()

    add_custom_target(${TARGET_NAME}
        COMMENT "Creating ${DIST_NAME}..."
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${DIST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E rm -f "${DIST_DIR}.zip"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/Client"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/Server"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Installer/Windows/License.txt" "${DIST_DIR}/License.txt"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_INSTALL_PREFIX}/Server/tt5srv.exe" "${DIST_DIR}/Server/"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_INSTALL_PREFIX}/Server/tt5svc.exe" "${DIST_DIR}/Server/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Windows/tt5srv_console.bat" "${DIST_DIR}/Server/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Windows/tt5svc_install.bat" "${DIST_DIR}/Server/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Windows/tt5svc_uninstall.bat" "${DIST_DIR}/Server/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_INSTALL_PREFIX}/Client/qtTeamTalk" "${DIST_DIR}/Client"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_INSTALL_PREFIX}/Client/qtTeamTalk/windeployqt" "${DIST_DIR}/Client"
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${DIST_DIR}/Client/windeployqt"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Client/Windows/TeamTalk5.ini" "${DIST_DIR}/Client/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Client/Windows/TeamTalk5.ini" "${DIST_DIR}/Client/TeamTalk5.ini.default"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_INSTALL_PREFIX}/Library/TeamTalk_DLL/TeamTalk5.dll" "${DIST_DIR}/Client/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TEAMTALK_ROOT}/Setup/Client/Sounds" "${DIST_DIR}/Client/Sounds"
        COMMAND ${CMAKE_COMMAND} -E copy ${EXTRA_DLLS} "${DIST_DIR}/Client/" || ${CMAKE_COMMAND} -E true
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Documentation/TeamTalk/output/TeamTalk5.chm" "${DIST_DIR}/Client/" || ${CMAKE_COMMAND} -E true
        COMMAND "${7Z_EXECUTABLE}" a -tzip "${DIST_DIR}.zip" "${DIST_DIR}"
        WORKING_DIRECTORY "${PORTABLE_OUTPUT_DIR}"
        VERBATIM
    )
endfunction()

function(create_windows_portable_pro TARGET_NAME ARCH)
    set(DIST_NAME "teamtalkpro-${PORTABLE_VERSION}-${ARCH}")
    set(DIST_DIR "${PORTABLE_OUTPUT_DIR}/${DIST_NAME}")

    add_custom_target(${TARGET_NAME}
        COMMENT "Creating ${DIST_NAME}..."
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${DIST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E rm -f "${DIST_DIR}.zip"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/Server"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_INSTALL_PREFIX}/Server/tt5prosrv.exe" "${DIST_DIR}/Server/"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_INSTALL_PREFIX}/Server/tt5prosvc.exe" "${DIST_DIR}/Server/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Windows/tt5prosrv_console.bat" "${DIST_DIR}/Server/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Windows/tt5prosvc_install.bat" "${DIST_DIR}/Server/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Windows/tt5prosvc_uninstall.bat" "${DIST_DIR}/Server/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Windows/README_professional" "${DIST_DIR}/README.txt"
        COMMAND "${7Z_EXECUTABLE}" a -tzip "${DIST_DIR}.zip" "${DIST_DIR}"
        WORKING_DIRECTORY "${PORTABLE_OUTPUT_DIR}"
        VERBATIM
    )
endfunction()

function(create_linux_portable TARGET_NAME DISTRO)
    set(DIST_NAME "teamtalk-${PORTABLE_VERSION}-${DISTRO}-x86_64")
    set(DIST_DIR "${PORTABLE_OUTPUT_DIR}/${DIST_NAME}")

    add_custom_target(${TARGET_NAME}
        COMMENT "Creating ${DIST_NAME}..."
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${DIST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E rm -f "${DIST_DIR}.tgz"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/client"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/server/init.d"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/server/systemd"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Installer/Windows/License.txt" "${DIST_DIR}/License.txt"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_INSTALL_PREFIX}/Server/tt5srv" "${DIST_DIR}/server/"
        COMMAND strip "${DIST_DIR}/server/tt5srv"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Linux/init.d/tt5server" "${DIST_DIR}/server/init.d/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Linux/init.d/README.md" "${DIST_DIR}/server/init.d/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Linux/systemd/tt5server.service" "${DIST_DIR}/server/systemd/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Linux/systemd/README.md" "${DIST_DIR}/server/systemd/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_INSTALL_PREFIX}/Client/qtTeamTalk" "${DIST_DIR}/client"
        COMMAND strip "${DIST_DIR}/client/teamtalk5"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Client/Linux/TeamTalk5.ini" "${DIST_DIR}/client/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Client/Linux/TeamTalk5.ini" "${DIST_DIR}/client/TeamTalk5.ini.default"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_INSTALL_PREFIX}/Library/TeamTalk_DLL/libTeamTalk5.so" "${DIST_DIR}/client/"
        COMMAND strip "${DIST_DIR}/client/libTeamTalk5.so"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${TEAMTALK_ROOT}/Setup/Client/Sounds" "${DIST_DIR}/client/sounds"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Client/Linux/README_debian" "${DIST_DIR}/client/README"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Client/Linux/run.sh" "${DIST_DIR}/client/run.sh"
        COMMAND chmod +x "${DIST_DIR}/client/run.sh"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/client/help"
        COMMAND bash -c "cp -r \"${TEAMTALK_ROOT}/Documentation/TeamTalk/output\"/* \"${DIST_DIR}/client/help/\" 2>/dev/null || true"
        COMMAND ${CMAKE_COMMAND} -E tar czf "${DIST_DIR}.tgz" "${DIST_NAME}"
        WORKING_DIRECTORY "${PORTABLE_OUTPUT_DIR}"
        VERBATIM
    )
endfunction()

function(create_linux_portable_pro TARGET_NAME DISTRO)
    set(DIST_NAME "teamtalkpro-${PORTABLE_VERSION}-${DISTRO}-x86_64")
    set(DIST_DIR "${PORTABLE_OUTPUT_DIR}/${DIST_NAME}")

    add_custom_target(${TARGET_NAME}
        COMMENT "Creating ${DIST_NAME}..."
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${DIST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E rm -f "${DIST_DIR}.tgz"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/server/init.d"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/server/systemd"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_INSTALL_PREFIX}/Server/tt5prosrv" "${DIST_DIR}/server/"
        COMMAND strip "${DIST_DIR}/server/tt5prosrv"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Linux/init.d/tt5proserver" "${DIST_DIR}/server/init.d/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Linux/init.d/README_pro.md" "${DIST_DIR}/server/init.d/README.md"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Linux/systemd/tt5proserver.service" "${DIST_DIR}/server/systemd/"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Linux/systemd/README_pro.md" "${DIST_DIR}/server/systemd/README.md"
        COMMAND ${CMAKE_COMMAND} -E copy "${TEAMTALK_ROOT}/Setup/Server/Linux/README_professional" "${DIST_DIR}/README"
        COMMAND ${CMAKE_COMMAND} -E tar czf "${DIST_DIR}.tgz" "${DIST_NAME}"
        WORKING_DIRECTORY "${PORTABLE_OUTPUT_DIR}"
        VERBATIM
    )
endfunction()

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        create_windows_portable(portable-win64 "win64")
        create_windows_portable_pro(portable-win64pro "win64")
    else()
        create_windows_portable(portable-win32 "win32")
        create_windows_portable_pro(portable-win32pro "win32")
    endif()
elseif(UNIX AND NOT APPLE)
    create_linux_portable(portable-ubuntu22 "ubuntu22")
    create_linux_portable(portable-ubuntu24 "ubuntu24")
    create_linux_portable(portable-rasp "raspios12")
    create_linux_portable_pro(portable-ubuntu22pro "ubuntu22")
    create_linux_portable_pro(portable-ubuntu24pro "ubuntu24")
    create_linux_portable_pro(portable-rasppro "raspios12")
endif()
