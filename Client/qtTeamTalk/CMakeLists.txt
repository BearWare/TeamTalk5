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

# Locate Qt Framework
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  find_package (Qt5 COMPONENTS Widgets Xml Network Multimedia TextToSpeech DBus X11Extras)
else()
  find_package (Qt5 COMPONENTS Widgets Xml Network Multimedia TextToSpeech)
endif()
# TextToSpeech was introduced in Qt 6.4
find_package (Qt6 COMPONENTS Widgets Xml Network Multimedia TextToSpeech)

if (Qt5_FOUND OR Qt6_FOUND)

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
  endif()

  if (Qt5_FOUND)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set (CMAKE_OSX_ARCHITECTURES x86_64)
    endif()
  endif()

  if (Qt6_FOUND)
    set (CMAKE_CXX_STANDARD 17)
    set (CMAKE_OSX_DEPLOYMENT_TARGET 11.0) # Qt 6.5.0 requires
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
    serverlogeventsmodel.h textmessagecontainer.h
    useraccountsmodel.h encryptionsetupdlg.h utiltt.h utilxml.h
    utilos.h serverdlg.h moveusersdlg.h useraccountdlg.h soundeventsmodel.h
    mytableview.h mytabwidget.h shortcutsmodel.h utilmedia.h

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
    serverlogeventsmodel.cpp textmessagecontainer.cpp
    useraccountsmodel.cpp encryptionsetupdlg.cpp utiltt.cpp utilxml.cpp
    utilos.cpp serverdlg.cpp moveusersdlg.cpp useraccountdlg.cpp soundeventsmodel.cpp
    mytableview.cpp mytabwidget.cpp shortcutsmodel.cpp utilmedia.cpp
    mainwindow.ui channel.ui preferences.ui
    serverlist.ui userinfo.ui bannedusers.ui useraccounts.ui
    serverproperties.ui uservideo.ui keycomp.ui textmessage.ui
    filetransfer.ui uservolume.ui changestatus.ui about.ui
    serverstats.ui onlineusers.ui mediastorage.ui userdesktop.ui
    desktopshare.ui streammediafile.ui videotext.ui desktopaccess.ui
    generatettfile.ui customvideofmt.ui bearwarelogindlg.ui
    audiopreprocessor.ui statusbardlg.ui encryptionsetupdlg.ui
    serverdlg.ui useraccountdlg.ui
    resources.qrc mainwindow.rc
    )

  if (Qt6_FOUND)
    set (TEAMTALK_LINK_FLAGS Qt6::Widgets Qt6::Xml Qt6::Network Qt6::Multimedia Qt6::TextToSpeech)
  else()
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      set (TEAMTALK_LINK_FLAGS Qt5::Widgets Qt5::Xml Qt5::Network Qt5::Multimedia Qt5::TextToSpeech Qt5::DBus Qt5::X11Extras -lX11 -lXss)
    else()
      set (TEAMTALK_LINK_FLAGS Qt5::Widgets Qt5::Xml Qt5::Network Qt5::Multimedia Qt5::TextToSpeech)
    endif()
  endif()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (TEAMTALK_TARGET TeamTalk5)
  elseif (MSVC)
    set (TEAMTALK_TARGET TeamTalk5)
  else()
    set (TEAMTALK_TARGET teamtalk5)
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

  option (BUILD_TEAMTALK_CLIENT_QTTEAMTALK "Build Qt TeamTalk client example" ON)

  if (BUILD_TEAMTALK_CLIENT_TOLK)
    list (APPEND TEAMTALK_LINK_FLAGS Tolk)
    install (FILES ${TOLK_DLL_FILES} DESTINATION Client/qtTeamTalk)
  endif()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_program(MACDEPLOYQT_EXE_PATH macdeployqt)
  elseif (MSVC)
    find_program(WINDEPLOYQT_EXE_PATH windeployqt)
  endif()

  if (BUILD_TEAMTALK_CLIENT_QTTEAMTALK)

    if (MSVC)
      add_executable (${TEAMTALK_TARGET} WIN32 ${QTTEAMTALK_SOURCES})

      option (BUILD_TEAMTALK_CLIENT_QT_DEPLOY "Deploy Qt shared libraries into 'windeployqt' subdirectory" ON)

      if (BUILD_TEAMTALK_CLIENT_QT_DEPLOY)

        if (WINDEPLOYQT_EXE_PATH-NOTFOUND)
          message(WARNING "Cannot deploy Qt to TeamTalk 5 Installer due to missing windeployqt")
        else()

          if (Qt6_FOUND)
            set (QT_DEPLOY_ARGS --verbose 2)
          else()
            # Qt5_FOUND
            set (QT_DEPLOY_ARGS --no-webkit2 --no-angle)
          endif()

          add_custom_target (QtTeamTalk5-windeploy-release ALL
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_SOURCE_DIR}/windeployqt"
            COMMAND ${WINDEPLOYQT_EXE_PATH} --no-quick-import --no-translations --no-system-d3d-compiler --no-virtualkeyboard ${QT_DEPLOY_ARGS} --no-opengl-sw --release --dir "${CMAKE_CURRENT_SOURCE_DIR}/windeployqt" "$<TARGET_FILE_DIR:${TEAMTALK_TARGET}>/$<TARGET_FILE_NAME:${TEAMTALK_TARGET}>"
            DEPENDS ${TEAMTALK_TARGET})

          add_custom_target (QtTeamTalk5-windeploy-debug
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_SOURCE_DIR}/windeployqt-debug"
            COMMAND ${WINDEPLOYQT_EXE_PATH} --no-quick-import --no-translations --no-system-d3d-compiler --no-virtualkeyboard ${QT_DEPLOY_ARGS} --no-opengl-sw --debug --dir "${CMAKE_CURRENT_SOURCE_DIR}/windeployqt-debug" "$<TARGET_FILE_DIR:${TEAMTALK_TARGET}>/$<TARGET_FILE_NAME:${TEAMTALK_TARGET}>"
            DEPENDS ${TEAMTALK_TARGET})

          # OpenSSL DLLs are not deployed by windeployqt.exe so we need to copy them manually.
          # Qt Maintenance Tool is able to download and install Qt's OpenSSL binaries. These files
          # will be located in <Qt install dir>/Tools/OpenSSL
          if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
            find_file (QT_OPENSSL_LIBCRYPTODLL_PATH NAMES libcrypto-1_1-x64.dll libcrypto-1_1.dll libcrypto.dll)
            find_file (QT_OPENSSL_LIBSSLDLL_PATH NAMES libssl-1_1-x64.dll libssl-1_1.dll libssl.dll)
          else()
            find_file (QT_OPENSSL_LIBCRYPTODLL_PATH NAMES libcrypto-1_1.dll libcrypto.dll)
            find_file (QT_OPENSSL_LIBSSLDLL_PATH NAMES libssl-1_1.dll libssl.dll)
          endif()
          if (QT_OPENSSL_LIBCRYPTODLL_PATH-NOTFOUND OR QT_OPENSSL_LIBSSLDLL_PATH-NOTFOUND)
            message (WARNING "Cannot find OpenSSL's libcrypto.dll and libssl.dll for Qt deployed DLLs")
          else()
            message ("OpenSSL libcrypto.dll for Qt deployment found here: ${QT_OPENSSL_LIBCRYPTODLL_PATH}")
            message ("OpenSSL libssl.dll for Qt deployment found here: ${QT_OPENSSL_LIBSSLDLL_PATH}")
            add_custom_target (QtTeamTalk5-openssl-deploy-release ALL
              COMMAND ${CMAKE_COMMAND} -E copy ${QT_OPENSSL_LIBCRYPTODLL_PATH} ${QT_OPENSSL_LIBSSLDLL_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/windeployqt"
              DEPENDS QtTeamTalk5-windeploy-release)
            add_custom_target (QtTeamTalk5-openssl-deploy-debug ALL
              COMMAND ${CMAKE_COMMAND} -E copy ${QT_OPENSSL_LIBCRYPTODLL_PATH} ${QT_OPENSSL_LIBSSLDLL_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/windeployqt-debug"
              DEPENDS QtTeamTalk5-windeploy-debug)
          endif()

          install (DIRECTORY windeployqt DESTINATION Client/qtTeamTalk/)
        endif()
      endif()
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      add_executable (${TEAMTALK_TARGET} ${QTTEAMTALK_SOURCES})
      set_target_properties(${TEAMTALK_TARGET} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
      target_link_options(${TEAMTALK_TARGET} PRIVATE -sectcreate __TEXT __info_plist ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
    else()
      add_executable (${TEAMTALK_TARGET} ${QTTEAMTALK_SOURCES})
    endif()

    if (BUILD_TEAMTALK_LIBRARY_DLL)
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

    # Build translations
    file (GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/languages/*.ts)
    set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/languages)
    if (Qt6_FOUND)
      find_package(Qt6LinguistTools)
      if (Qt6LinguistTools_FOUND)
        # This generates target "update_translations" and "${TEAMTALK_TARGET}_lupdate"
        qt_add_lupdate (${TEAMTALK_TARGET} TS_FILES ${TS_FILES} OPTIONS -no-obsolete)
        qt6_add_translation (QM_FILES ${TS_FILES})
        add_custom_target(QtTeamTalk5-Translations DEPENDS ${QM_FILES})
        install (FILES ${QM_FILES} DESTINATION Client/qtTeamTalk/languages)
      endif()
    else()
      find_package(Qt5LinguistTools)
      if (Qt5LinguistTools_FOUND)
        qt5_add_translation (QM_FILES ${TS_FILES})
        add_custom_target(QtTeamTalk5-Translations DEPENDS ${QM_FILES})
        install (FILES ${QM_FILES} DESTINATION Client/qtTeamTalk/languages)
      endif()
    endif()

    if (Qt5LinguistTools_FOUND OR Qt6LinguistTools_FOUND)
      add_dependencies(${TEAMTALK_TARGET} QtTeamTalk5-Translations)
    endif()

    set_property(TARGET ${TEAMTALK_TARGET} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Library/TeamTalk_DLL)

    set_output_dir(${TEAMTALK_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/")

    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

      option (BUILD_TEAMTALK_CLIENT_QTTEAMTALK_PRODUCT "Build Qt TeamTalk 5 Installer" OFF)

      if (BUILD_TEAMTALK_CLIENT_QTTEAMTALK_PRODUCT)

        if (MACDEPLOYQT_EXECUTABLE-NOTFOUND)
          message(WARNING "Cannot deploy Qt to TeamTalk 5 Installer due to missing macdeployqt")
        else()

          # Applies 'macdeployqt' for installing Qt frameworks, i.e. folder: TeamTalk5.app/Contents/Frameworks
          add_custom_command(TARGET ${TEAMTALK_TARGET} POST_BUILD
            COMMAND ${MACDEPLOYQT_EXE_PATH} $<TARGET_FILE_DIR:${TEAMTALK_TARGET}>/../.. -always-overwrite
            COMMENT "Running macdeployqt on ${TEAMTALK_TARGET}.app")

          # Prepare deployed TeamTalk5.app folder with client resources, i.e. language files, sounds, etc.
          add_custom_target(QtTeamTalk5ClientProduct ALL
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
          add_custom_target(QtTeamTalk5ServerProduct ALL
            # Copy server
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Server
            COMMAND ${CMAKE_COMMAND} -E copy ${TEAMTALK_ROOT}/Server/tt5srv ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app/Contents/Server
            COMMENT "Applying TeamTalk server to TeamTalk 5 Installer for macOS"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS tt5srv QtTeamTalk5ClientProduct)

          option (BUILD_TEAMTALK_CLIENT_QTTEAMTALK_PRODUCT_CODESIGN "Sign Qt TeamTalk 5 Install Package" OFF)

          if (BUILD_TEAMTALK_CLIENT_QTTEAMTALK_PRODUCT_CODESIGN)

            # Build TeamTalk 5 installer for Apple AppStore, i.e. without server application
            add_custom_target(QtTeamTalk5Product ALL
              # Run code sign
              COMMAND codesign --deep -f --sign "Apple Distribution: Bjoern Rasmussen" --entitlements ${CMAKE_CURRENT_SOURCE_DIR}/Entitlements.plist ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app
              # Run productbuild
              COMMAND productbuild --component ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app /Applications --sign "3rd Party Mac Developer Installer: Bjoern Rasmussen" ${TEAMTALK_TARGET}_AppStore_Setup.pkg
              COMMAND productsign --sign "3rd Party Mac Developer Installer: Bjoern Rasmussen" ${TEAMTALK_TARGET}_AppStore_Setup.pkg ${TEAMTALK_TARGET}_AppStore_Setup_Signed.pkg
              COMMENT "Building TeamTalk 5 Installer for AppStore"
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              BYPRODUCTS ${TEAMTALK_TARGET}_AppStore_Setup_Signed.pkg
              DEPENDS QtTeamTalk5ClientProduct)

            install (FILES ${TEAMTALK_TARGET}_AppStore_Setup_Signed.pkg DESTINATION Client/qtTeamTalk)

            # Build TeamTalk 5 install for macOS with server application
            add_custom_target(QtTeamTalk5WithServerProduct ALL
              # Run code sign
              COMMAND codesign --deep -f --sign "Apple Distribution: Bjoern Rasmussen" ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app
              # Run productbuild
              COMMAND productbuild --component ${CMAKE_CURRENT_SOURCE_DIR}/${TEAMTALK_TARGET}.app /Applications --sign "3rd Party Mac Developer Installer: Bjoern Rasmussen" ${TEAMTALK_TARGET}_Setup.pkg
              COMMAND productsign --sign "3rd Party Mac Developer Installer: Bjoern Rasmussen" ${TEAMTALK_TARGET}_Setup.pkg ${TEAMTALK_TARGET}_Setup_Signed.pkg
              COMMENT "Building TeamTalk 5 Installer for macOS with server"
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              BYPRODUCTS ${TEAMTALK_TARGET}_Setup_Signed.pkg
              DEPENDS QtTeamTalk5Product QtTeamTalk5ServerProduct)

            install (FILES ${TEAMTALK_TARGET}_Setup_Signed.pkg DESTINATION Client/qtTeamTalk)
          endif()

        endif()

      endif()
    endif()

    install (TARGETS ${TEAMTALK_TARGET} DESTINATION Client/qtTeamTalk)

  endif()

endif()
