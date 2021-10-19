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

if (MSVC)
  option (QT_BUILD_QTFRAMEWORK "Build Qt Framework for Windows" OFF)

  if (QT_BUILD_QTFRAMEWORK)
    add_subdirectory(qt)
  endif()

  option (BUILD_TEAMTALK_CLIENT_QT_MULTITHREADED "Build TeamTalk Qt-based clients as Multi-Threaded (/MT) instead of Multi-Threaded DLL (/MD)" ON)
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

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  find_package (Qt5 COMPONENTS Widgets Xml Network Multimedia TextToSpeech X11Extras)
else()
  find_package (Qt5 COMPONENTS Widgets Xml Network Multimedia TextToSpeech)
endif()
find_package (Qt6 COMPONENTS Widgets Xml Network Multimedia)

if (Qt5_FOUND OR Qt6_FOUND)

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
    onlineusersmodel.h chatlineedit.h mediastoragedlg.h
    desktopgridwidget.h userdesktopwidget.h gridwidget.h
    userimagewidget.h userdesktopdlg.h desktopsharedlg.h
    sendtextedit.h streammediafiledlg.h videotextdlg.h
    desktopaccessdlg.h appinfo.h settings.h generatettfiledlg.h
    customvideofmtdlg.h license.h bearwarelogindlg.h
    audiopreprocessordlg.h ttseventsmodel.h
    statusbardlg.h statusbareventsmodel.h mycombobox.h

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

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Don't set to to "TeamTalk5" on macOS since we still build with
    # QMake
    set (TEAMTALK_TARGET TeamTalk)
    set (TEAMTALKPRO_TARGET TeamTalkPro)
  elseif (MSVC)
    set (TEAMTALK_TARGET TeamTalk5)
    set (TEAMTALKPRO_TARGET TeamTalk5Pro)
  else()
    set (TEAMTALK_TARGET teamtalk5)
    set (TEAMTALKPRO_TARGET teamtalk5pro)
  endif()

  if (Qt6_FOUND)
    set (TEAMTALK_LINK_FLAGS Qt6::Widgets Qt6::Xml Qt6::Network)
  else()
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      set (TEAMTALK_LINK_FLAGS Qt5::Widgets Qt5::Xml Qt5::Network Qt5::Multimedia Qt5::TextToSpeech Qt5::X11Extras -lX11)
    else()
      set (TEAMTALK_LINK_FLAGS Qt5::Widgets Qt5::Xml Qt5::Network Qt5::Multimedia Qt5::TextToSpeech)
    endif()
  endif()

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_library (CARBON_LIBRARY Carbon)
    find_library (IOKIT_LIBRARY IOKit)
    list (APPEND TEAMTALK_LINK_FLAGS ${CARBON_LIBRARY} ${IOKIT_LIBRARY})

    set (MACOSX_BUNDLE TRUE)
    set_source_files_properties(images/teamtalk.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  endif()

  option (BUILD_TEAMTALK_CLIENT_QT_EXAMPLE "Build Qt Client for TeamTalk 5 Standard Edition" ON)
  option (BUILD_TEAMTALK_PROCLIENT_QT_EXAMPLE "Build Qt Client for TeamTalk 5 Professional Edition" ON)

  if (BUILD_TEAMTALK_CLIENT_TOLK)
    list (APPEND TEAMTALK_LINK_FLAGS Tolk)
    install (FILES ${TOLK_DLL_FILES} DESTINATION Client/qtTeamTalk)
  endif()

  if (BUILD_TEAMTALK_CLIENT_QT_EXAMPLE)

    add_executable (${TEAMTALK_TARGET} ${QTTEAMTALK_SOURCES})

    if (BUILD_TEAMTALK_DLL)
      target_link_libraries (${TEAMTALK_TARGET} ${TEAMTALK_LINK_FLAGS} TeamTalk5DLL)
    else()
      find_library(TEAMTALK5_LIBRARY TeamTalk5 PATHS ../../Library/TeamTalk_DLL)
      if (NOT TEAMTALK5_LIBRARY)
          message("Cannot find TeamTalk5 shared library in ../../Library/TeamTalk_DLL")
          target_link_libraries (${TEAMTALK_TARGET} ${TEAMTALK_LINK_FLAGS} TeamTalk5)
      else()
          target_link_libraries (${TEAMTALK_TARGET} ${TEAMTALK_LINK_FLAGS} ${TEAMTALK5_LIBRARY})
      endif()
    endif()

    target_include_directories (${TEAMTALK_TARGET} PUBLIC ../../Library/TeamTalk_DLL ./)

    if (BUILD_TEAMTALK_CLIENT_TOLK)
      target_compile_options(${TEAMTALK_TARGET} PUBLIC -DENABLE_TOLK -D_CRT_SECURE_NO_WARNINGS)
    elseif (MSVC)
      target_compile_options(${TEAMTALK_TARGET} PUBLIC -D_CRT_SECURE_NO_WARNINGS)
    endif()

    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set_target_properties(${TEAMTALK_TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST Info.plist)
    endif()
    set_output_dir(${TEAMTALK_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/")

    install (TARGETS ${TEAMTALK_TARGET} DESTINATION Client/qtTeamTalk)

  endif()

  if (BUILD_TEAMTALK_PROCLIENT_QT_EXAMPLE)

    add_executable (${TEAMTALKPRO_TARGET} ${QTTEAMTALK_SOURCES})

    if (BUILD_TEAMTALK_PRODLL)
      target_link_libraries (${TEAMTALKPRO_TARGET} ${TEAMTALK_LINK_FLAGS} TeamTalk5ProDLL)
    else()
      find_library(TEAMTALK5PRO_LIBRARY TeamTalk5Pro PATHS ../../Library/TeamTalk_DLL)
      if (NOT TEAMTALK5PRO_LIBRARY)
          message("Cannot find TeamTalk5Pro shared library in ../../Library/TeamTalk_DLL")
          target_link_libraries (${TEAMTALKPRO_TARGET} ${TEAMTALK_LINK_FLAGS} TeamTalk5Pro)
      else()
          target_link_libraries (${TEAMTALKPRO_TARGET} ${TEAMTALK_LINK_FLAGS} ${TEAMTALK5PRO_LIBRARY})
      endif()
    endif()

    target_include_directories (${TEAMTALKPRO_TARGET} PUBLIC ../../Library/TeamTalk_DLL ./)

    if (BUILD_TEAMTALK_CLIENT_TOLK)
      target_compile_options(${TEAMTALKPRO_TARGET} PUBLIC -DENABLE_TOLK -D_CRT_SECURE_NO_WARNINGS -DENABLE_TEAMTALKPRO)
    elseif (MSVC)
      target_compile_options(${TEAMTALKPRO_TARGET} PUBLIC -D_CRT_SECURE_NO_WARNINGS -DENABLE_TEAMTALKPRO)
    else()
      target_compile_options(${TEAMTALKPRO_TARGET} PUBLIC -DENABLE_TEAMTALKPRO)
    endif()

    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set_target_properties(${TEAMTALKPRO_TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST Info.plist)
    endif()
    set_output_dir(${TEAMTALKPRO_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/")

    install (TARGETS ${TEAMTALKPRO_TARGET} DESTINATION Client/qtTeamTalk)
  endif()

  find_package(Qt5LinguistTools)
  if (Qt5LinguistTools_FOUND)
    file (GLOB TS_FILES ${CMAKE_CURRENT_LIST_DIR}/languages/*.ts)
    set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_LIST_DIR}/languages)
    qt5_add_translation (QM_FILES ${TS_FILES})
    add_custom_target(QtTeamTalk5-Translations DEPENDS ${QM_FILES})
    install (FILES ${QM_FILES} DESTINATION Client/qtTeamTalk/languages)
    if (BUILD_TEAMTALK_CLIENT_QT_EXAMPLE)
      add_dependencies(${TEAMTALK_TARGET} QtTeamTalk5-Translations)
    endif()
    if (BUILD_TEAMTALK_PROCLIENT_QT_EXAMPLE)
      add_dependencies(${TEAMTALKPRO_TARGET} QtTeamTalk5-Translations)
    endif()
  endif()


endif()
