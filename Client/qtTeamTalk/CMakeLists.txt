cmake_minimum_required(VERSION 3.1)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

project (QtTeamTalk5 VERSION 1.0.0 LANGUAGES CXX)

function(set_output_dir target dir)
  set_target_properties (${target} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${dir}
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${dir}
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${dir}
    RUNTIME_OUTPUT_DIRECTORY ${dir}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${dir}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${dir}
    ARCHIVE_OUTPUT_DIRECTORY ${dir}
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${dir}
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${dir})
endfunction()

# Option to build Qt Framework on Windows. For CMake to find Qt set up
# environment variable QTDIR to point to QT_INSTALL_PREFIX defined in
# qt/CMakeLists.txt
option (QT_BUILD_QTFRAMEWORK "Build Qt Framework" OFF)
if (QT_BUILD_QTFRAMEWORK)
  add_subdirectory(qt)
endif()

# Handle QMake support for Visual Studio
if (MSVC)
  option (BUILD_TEAMTALK_CLIENT_QT_MULTITHREADED "Build TeamTalk Qt-based clients as Multi-Threaded (/MT) instead of Multi-Threaded DLL (/MD)" OFF)
  if (BUILD_TEAMTALK_CLIENT_QT_MULTITHREADED)
    foreach (flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if (${flag_var} MATCHES "/MD")
        STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif()
    endforeach(flag_var)
  endif()

  # QMake must have generated .vcxproj file prior
  option (BUILD_TEAMTALK_CLIENT_QT_EXAMPLE_WITH_QMAKE "Build TeamTalk Qt client example using QMake" OFF)

  if (BUILD_TEAMTALK_CLIENT_QT_EXAMPLE_WITH_QMAKE)
    include_external_msproject (QtTeamTalk5 ${CMAKE_CURRENT_LIST_DIR}/TeamTalk5.vcxproj)

    if (BUILD_TEAMTALK_DLL)
      add_dependencies (QtTeamTalk5 TeamTalk5DLL)
    endif()
  endif()

  option (BUILD_TEAMTALK_PROCLIENT_QT_EXAMPLE_WITH_QMAKE "Build TeamTalk Qt client example using QMake" OFF)

  if (BUILD_TEAMTALK_PROCLIENT_QT_EXAMPLE_WITH_QMAKE)
    include_external_msproject (QtTeamTalk5Pro ${CMAKE_CURRENT_LIST_DIR}/TeamTalk5Pro.vcxproj)

    if (BUILD_TEAMTALK_PRODLL)
      add_dependencies (QtTeamTalk5Pro TeamTalk5ProDLL)
    endif()
  endif()

endif()

# Locate Qt Framework
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  find_package (Qt5 COMPONENTS Widgets Xml Network Multimedia TextToSpeech X11Extras)
else()
  find_package (Qt5 COMPONENTS Widgets Xml Network Multimedia TextToSpeech)
endif()
find_package (Qt6 COMPONENTS Widgets Xml Network Multimedia)

if (Qt5_FOUND OR Qt6_FOUND)

  if (Qt5_FOUND)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set (CMAKE_OSX_ARCHITECTURES x86_64)
    endif()
  endif()

  if (Qt6_FOUND)
    set (CMAKE_CXX_STANDARD 17)
  else()
    set (CMAKE_CXX_STANDARD 11)
  endif()

  set (CMAKE_CXX_STANDARD_REQUIRED ON)

  set (CMAKE_AUTOMOC ON)
  set (CMAKE_AUTORCC ON)
  set (CMAKE_AUTOUIC ON)

  set (QTTEAMTALK_SOURCES
    mainwindow.h preferencesdlg.h uservideowidget.h
    channelstree.h channeldlg.h userinfodlg.h bannedusersdlg.h
    useraccountsdlg.h videogridwidget.h uservideodlg.h
    serverpropertiesdlg.h keycompdlg.h serverlistdlg.h common.h
    textmessagedlg.h chattextedit.h filesmodel.h filetransferdlg.h
    filesview.h uservolumedlg.h changestatusdlg.h aboutdlg.h
    chatlineedit.h serverstatsdlg.h onlineusersdlg.h
    onlineusersmodel.h mediastoragedlg.h
    desktopgridwidget.h userdesktopwidget.h gridwidget.h
    userimagewidget.h userdesktopdlg.h desktopsharedlg.h
    sendtextedit.h streammediafiledlg.h videotextdlg.h
    desktopaccessdlg.h appinfo.h settings.h generatettfiledlg.h
    customvideofmtdlg.h license.h bearwarelogindlg.h
    audiopreprocessordlg.h ttseventsmodel.h
    statusbardlg.h statusbareventsmodel.h mycombobox.h
    utilsound.h utilvideo.h utiltts.h utilui.h utilhotkey.h
    serverlogeventsmodel.h mytreeview.h

    main.cpp mainwindow.cpp preferencesdlg.cpp uservideowidget.cpp
    channelstree.cpp channeldlg.cpp userinfodlg.cpp
    bannedusersdlg.cpp useraccountsdlg.cpp videogridwidget.cpp
    uservideodlg.cpp serverpropertiesdlg.cpp keycompdlg.cpp
    serverlistdlg.cpp common.cpp textmessagedlg.cpp chattextedit.cpp
    filesmodel.cpp filetransferdlg.cpp filesview.cpp
    uservolumedlg.cpp changestatusdlg.cpp aboutdlg.cpp
    chatlineedit.cpp serverstatsdlg.cpp onlineusersdlg.cpp
    onlineusersmodel.cpp mediastoragedlg.cpp desktopgridwidget.cpp
    userdesktopwidget.cpp gridwidget.cpp userimagewidget.cpp
    userdesktopdlg.cpp desktopsharedlg.cpp sendtextedit.cpp
    streammediafiledlg.cpp videotextdlg.cpp desktopaccessdlg.cpp
    generatettfiledlg.cpp customvideofmtdlg.cpp
    bearwarelogindlg.cpp audiopreprocessordlg.cpp ttseventsmodel.cpp
    statusbardlg.cpp statusbareventsmodel.cpp mycombobox.cpp
    utilsound.cpp utilvideo.cpp utiltts.cpp utilui.cpp utilhotkey.cpp
    serverlogeventsmodel.cpp mytreeview.cpp

    mainwindow.ui channel.ui preferences.ui
    serverlist.ui userinfo.ui bannedusers.ui useraccounts.ui
    serverproperties.ui uservideo.ui keycomp.ui textmessage.ui
    filetransfer.ui uservolume.ui changestatus.ui about.ui
    serverstats.ui onlineusers.ui mediastorage.ui userdesktop.ui
    desktopshare.ui streammediafile.ui videotext.ui desktopaccess.ui
    generatettfile.ui customvideofmt.ui bearwarelogindlg.ui
    audiopreprocessor.ui statusbardlg.ui
    resources.qrc mainwindow.rc
    )

  find_package(Qt5LinguistTools)
  if (Qt5LinguistTools_FOUND)
    file (GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/languages/*.ts)
    set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/languages)
    qt5_add_translation (QM_FILES ${TS_FILES})
    add_custom_target(QtTeamTalk5-Translations DEPENDS ${QM_FILES})
    install (FILES ${QM_FILES} DESTINATION Client/qtTeamTalk/languages)
  endif()

  if (Qt6_FOUND)
    set (TEAMTALK_LINK_FLAGS Qt6::Widgets Qt6::Xml Qt6::Network Qt6::Multimedia)
  else()
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      set (TEAMTALK_LINK_FLAGS Qt5::Widgets Qt5::Xml Qt5::Network Qt5::Multimedia Qt5::TextToSpeech Qt5::X11Extras -lX11)
    else()
      set (TEAMTALK_LINK_FLAGS Qt5::Widgets Qt5::Xml Qt5::Network Qt5::Multimedia Qt5::TextToSpeech)
    endif()
  endif()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Don't set to to "TeamTalk5" on macOS since we still build with
    # QMake
    set (TEAMTALK_TARGET TeamTalk5)
    set (TEAMTALKPRO_TARGET TeamTalk5Pro)
  elseif (MSVC)
    set (TEAMTALK_TARGET TeamTalk5)
    set (TEAMTALKPRO_TARGET TeamTalk5Pro)
  else()
    set (TEAMTALK_TARGET teamtalk5)
    set (TEAMTALKPRO_TARGET teamtalk5pro)
  endif()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_library (CARBON_LIBRARY Carbon)
    find_library (IOKIT_LIBRARY IOKit)
    list (APPEND TEAMTALK_LINK_FLAGS ${CARBON_LIBRARY} ${IOKIT_LIBRARY})

    set (MACOSX_BUNDLE_ICON_FILE teamtalk.icns)
    set (TEAMTALK_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/images/teamtalk.icns)
    list (APPEND QTTEAMTALK_SOURCES ${TEAMTALK_ICON_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist ${CMAKE_CURRENT_SOURCE_DIR}/Entitlements.plist)
    set_source_files_properties (${TEAMTALK_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  endif()

  option (BUILD_TEAMTALK_CLIENT_QT_EXAMPLE "Build Qt Client for TeamTalk 5 Standard Edition" ON)
  option (BUILD_TEAMTALK_PROCLIENT_QT_EXAMPLE "Build Qt Client for TeamTalk 5 Professional Edition" ON)

  if (BUILD_TEAMTALK_CLIENT_TOLK)
    list (APPEND TEAMTALK_LINK_FLAGS Tolk)
    install (FILES ${TOLK_DLL_FILES} DESTINATION Client/qtTeamTalk)
  endif()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_program(MACDEPLOYQT_EXE_PATH macdeployqt)
  endif()

  if (BUILD_TEAMTALK_CLIENT_QT_EXAMPLE)

    if (MSVC)
      add_executable (${TEAMTALK_TARGET} WIN32 ${QTTEAMTALK_SOURCES})
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      add_executable (${TEAMTALK_TARGET} ${QTTEAMTALK_SOURCES})
      set_target_properties(${TEAMTALK_TARGET} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
      target_link_options(${TEAMTALK_TARGET} PRIVATE -sectcreate __TEXT __info_plist ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
    else()
      add_executable (${TEAMTALK_TARGET} ${QTTEAMTALK_SOURCES})
    endif()

    if (BUILD_TEAMTALK_DLL)
      target_include_directories (${TEAMTALK_TARGET} PUBLIC ./)
      target_link_libraries (${TEAMTALK_TARGET} ${TEAMTALK_LINK_FLAGS} TeamTalk5DLL)
    else()
      target_include_directories (${TEAMTALK_TARGET} PUBLIC ../../Library/TeamTalk_DLL ./)
      find_library(TEAMTALK5_LIBRARY TeamTalk5 PATHS ../../Library/TeamTalk_DLL)
      if (NOT TEAMTALK5_LIBRARY)
        message(WARNING "Cannot find TeamTalk5 shared library in ../../Library/TeamTalk_DLL")
      else()
        target_link_libraries (${TEAMTALK_TARGET} ${TEAMTALK_LINK_FLAGS} ${TEAMTALK5_LIBRARY})
      endif()
    endif()

    if (BUILD_TEAMTALK_CLIENT_TOLK)
      target_compile_options(${TEAMTALK_TARGET} PUBLIC -DUNICODE -D_UNICODE -DENABLE_TOLK -D_CRT_SECURE_NO_WARNINGS)
    elseif (MSVC)
      target_compile_options(${TEAMTALK_TARGET} PUBLIC -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS)
    endif()

    add_dependencies(${TEAMTALK_TARGET} QtTeamTalk5-Translations)

    set_property(TARGET ${TEAMTALK_TARGET} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/TeamTalk_DLL)

    set_output_dir(${TEAMTALK_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/")

    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

      option (BUILD_TEAMTALK_CLIENT_QT_EXAMPLE_PRODUCT "Build Qt TeamTalk 5 Installer" OFF)

      if (BUILD_TEAMTALK_CLIENT_QT_EXAMPLE_PRODUCT)

        if (MACDEPLOYQT_EXECUTABLE-NOTFOUND)
          message(WARNING "Cannot Build Qt TeamTalk 5 Installer due to missing macdeployqt")
        else()

          # Applies 'macdeployqt' for installing Qt frameworks, i.e. folder: TeamTalk5.app/Contents/Frameworks
          add_custom_command(TARGET ${TEAMTALK_TARGET} POST_BUILD
            COMMAND ${MACDEPLOYQT_EXE_PATH} $<TARGET_FILE_DIR:${TEAMTALK_TARGET}>/../.. -always-overwrite
            COMMENT "Running macdeployqt on ${TEAMTALK_TARGET}.app")

          # Prepare deployed TeamTalk5.app folder with client resources, i.e. language files, sounds, etc.
          add_custom_target(QtTeamTalk5ClientProduct
            # Copy language files
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Resources/Languages
            COMMAND ${CMAKE_COMMAND} -E copy ${QM_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Resources/Languages
            # Copy sounds
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Resources/Sounds
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${TEAMTALK_ROOT}/Setup/Client/Sounds ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Resources/Sounds
            # Copy help
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Resources/Help
            COMMAND ${CMAKE_COMMAND} -E copy ${TEAMTALK_ROOT}/Documentation/TeamTalk/output/* ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Resources/Help
            # Copy config file
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Resources/Config
            COMMAND ${CMAKE_COMMAND} -E copy ${TEAMTALK_ROOT}/Setup/Client/Mac/TeamTalk5.ini ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Resources/Config/TeamTalk5.ini
            COMMENT "Applying TeamTalk client to TeamTalk 5 Installer for macOS"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS QtTeamTalk5-Translations ${TEAMTALK_TARGET} TeamTalk-dox)

          # Copies TeamTalk server to TeamTalk5.app folder, i.e. TeamTalk5.app/Contents/Server
          add_custom_target(QtTeamTalk5ServerProduct
            # Copy server
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Server
            COMMAND ${CMAKE_COMMAND} -E copy ${TEAMTALK_ROOT}/Server/tt5srv ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Server
            COMMENT "Applying TeamTalk server to TeamTalk 5 Installer for macOS"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS tt5srv QtTeamTalk5ClientProduct)

          # Build TeamTalk 5 installer for Apple AppStore, i.e. without server application
          add_custom_target(QtTeamTalk5Product ALL
            # Run code sign
            COMMAND codesign --deep -f --sign "Apple Distribution: Bjoern Rasmussen" --entitlements ${CMAKE_CURRENT_SOURCE_DIR}/Entitlements.plist ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app
            # Run productbuild
            COMMAND productbuild --component ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app /Applications --sign "3rd Party Mac Developer Installer: Bjoern Rasmussen" ${TEAMTALK_TARGET}_AppStore_Setup.pkg
            COMMAND productsign --sign "3rd Party Mac Developer Installer: Bjoern Rasmussen" ${TEAMTALK_TARGET}_AppStore_Setup.pkg ${TEAMTALK_TARGET}_AppStore_Setup_Signed.pkg
            COMMENT "Building TeamTalk 5 Installer for AppStore"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS QtTeamTalk5ClientProduct)

          # Build TeamTalk 5 install for macOS with server application
          add_custom_target(QtTeamTalk5WithServerProduct ALL
            # Run code sign
            COMMAND codesign --deep -f --sign "Apple Distribution: Bjoern Rasmussen" ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app
            # Run productbuild
            COMMAND productbuild --component ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app /Applications --sign "3rd Party Mac Developer Installer: Bjoern Rasmussen" ${TEAMTALK_TARGET}_Setup.pkg
            COMMAND productsign --sign "3rd Party Mac Developer Installer: Bjoern Rasmussen" ${TEAMTALK_TARGET}_Setup.pkg ${TEAMTALK_TARGET}_Setup_Signed.pkg
            COMMENT "Building TeamTalk 5 Installer for macOS with server"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS QtTeamTalk5Product QtTeamTalk5ServerProduct)

        endif()

      endif()
    endif()

    install (TARGETS ${TEAMTALK_TARGET} DESTINATION Client/qtTeamTalk)

  endif()

  if (BUILD_TEAMTALK_PROCLIENT_QT_EXAMPLE)

    if (MSVC)
      add_executable (${TEAMTALKPRO_TARGET} WIN32 ${QTTEAMTALK_SOURCES})
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      add_executable (${TEAMTALKPRO_TARGET} MACOSX_BUNDLE ${QTTEAMTALK_SOURCES})
    else()
      add_executable (${TEAMTALKPRO_TARGET} ${QTTEAMTALK_SOURCES})
    endif()

    if (BUILD_TEAMTALK_PRODLL)
      target_include_directories (${TEAMTALKPRO_TARGET} PUBLIC ./)
      target_link_libraries (${TEAMTALKPRO_TARGET} ${TEAMTALK_LINK_FLAGS} TeamTalk5ProDLL)
    else()
      target_include_directories (${TEAMTALKPRO_TARGET} PUBLIC ../../Library/TeamTalk_DLL ./)
      find_library(TEAMTALK5PRO_LIBRARY TeamTalk5Pro PATHS ../../Library/TeamTalk_DLL)
      if (NOT TEAMTALK5PRO_LIBRARY)
        message(WARNING "Cannot find TeamTalk5Pro shared library in ../../Library/TeamTalk_DLL")
      else()
        target_link_libraries (${TEAMTALKPRO_TARGET} ${TEAMTALK_LINK_FLAGS} ${TEAMTALK5PRO_LIBRARY})
      endif()
    endif()

    if (BUILD_TEAMTALK_CLIENT_TOLK)
      target_compile_options(${TEAMTALKPRO_TARGET} PUBLIC -DUNICODE -D_UNICODE -DENABLE_TOLK -D_CRT_SECURE_NO_WARNINGS -DENABLE_TEAMTALKPRO)
    elseif (MSVC)
      target_compile_options(${TEAMTALKPRO_TARGET} PUBLIC -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -DENABLE_TEAMTALKPRO)
    else()
      target_compile_options(${TEAMTALKPRO_TARGET} PUBLIC -DENABLE_TEAMTALKPRO)
    endif()

    add_dependencies(${TEAMTALKPRO_TARGET} QtTeamTalk5-Translations)

    set_property(TARGET ${TEAMTALKPRO_TARGET} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/TeamTalk_DLL)

    set_output_dir(${TEAMTALKPRO_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/")

    install (TARGETS ${TEAMTALKPRO_TARGET} DESTINATION Client/qtTeamTalk)
  endif()

endif()
