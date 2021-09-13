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

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  find_package (Qt5 COMPONENTS Widgets Xml Network Multimedia TextToSpeech X11Extras)
else()
  find_package (Qt5 COMPONENTS Widgets Xml Network Multimedia TextToSpeech)
endif()
find_package (Qt6 COMPONENTS Widgets Xml Network)

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
    set (TEAMTALK_TARGET TeamTalk)
    set (TEAMTALKPRO_TARGET TeamTalkPro)
  elseif (MSVC)
    set (TEAMTALK_TARGET TeamTalk)
    set (TEAMTALKPRO_TARGET TeamTalkPro)
  else()
    set (TEAMTALK_TARGET teamtalk)
    set (TEAMTALKPRO_TARGET teamtalkpro)
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
  option (BUILD_TEAMTALKPRO_CLIENT_QT_EXAMPLE "Build Qt Client for TeamTalk 5 Professional Edition" ON)

  # Try locating Tolk screen-reader abstraction layer
  if (${CMAKE_GENERATOR_PLATFORM} MATCHES "x64")
    find_library (TOLK_LIBRARY NAMES Tolk PATHS tolk/bin/x64)
    message ("Looking for Tolk in tolk/bin/x64")
  else()
    find_library (TOLK_LIBRARY NAMES Tolk PATHS tolk/bin/win32)
    message ("Looking for Tolk in tolk/bin/win32")
  endif()

  if (NOT TOLK_LIBRARY)
    message ("Cannot find Tolk shared library in tolk/bin/$(PlatformName). Building without Tolk.")
  else()
    message("Tolk lib: ${TOLK_LIBRARY}")
    list (APPEND TEAMTALK_LINK_FLAGS ${TOLK_LIBRARY})
  endif()

  if (BUILD_TEAMTALK_CLIENT_QT_EXAMPLE)

    add_executable (${TEAMTALK_TARGET} ${QTTEAMTALK_SOURCES})

    if (BUILD_TEAMTALK5DLL)
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

    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set_target_properties(${TEAMTALK_TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST Info.plist)
    endif()
    set_output_dir(${TEAMTALK_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/")
  endif()

  if (BUILD_TEAMTALKPRO_CLIENT_QT_EXAMPLE)

    add_executable (${TEAMTALKPRO_TARGET} ${QTTEAMTALK_SOURCES})

    if (BUILD_TEAMTALK5PRODLL)
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

    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set_target_properties(${TEAMTALKPRO_TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST Info.plist)
    endif()
    set_output_dir(${TEAMTALKPRO_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/")
  endif()

endif()
