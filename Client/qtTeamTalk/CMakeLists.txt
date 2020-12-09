cmake_minimum_required (VERSION 3.16.0)

project (QtTeamTalk5 VERSION 1.0.0 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)

find_package (Qt6 COMPONENTS Widgets Xml Network REQUIRED)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (TEAMTALK_TARGET TeamTalk)
elseif (MSVC)
    set (TEAMTALK_TARGET TeamTalk)
else()
    set (TEAMTALK_TARGET teamtalk)
endif()

add_executable (${TEAMTALK_TARGET}
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
  customvideofmtdlg.h weblogindlg.h license.h bearwarelogindlg.h
  audiopreprocessordlg.h
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
  generatettfiledlg.cpp customvideofmtdlg.cpp weblogindlg.cpp
  bearwarelogindlg.cpp audiopreprocessordlg.cpp
  mainwindow.ui channel.ui preferences.ui
  serverlist.ui userinfo.ui bannedusers.ui useraccounts.ui
  serverproperties.ui uservideo.ui keycomp.ui textmessage.ui
  filetransfer.ui uservolume.ui changestatus.ui about.ui
  serverstats.ui onlineusers.ui mediastorage.ui userdesktop.ui
  desktopshare.ui streammediafile.ui videotext.ui desktopaccess.ui
  generatettfile.ui customvideofmt.ui weblogin.ui bearwarelogindlg.ui
  audiopreprocessor.ui  
  resources.qrc mainwindow.rc
)

target_include_directories (${TEAMTALK_TARGET} PRIVATE ../../Library/TeamTalk_DLL ./)
target_link_directories (${TEAMTALK_TARGET} PRIVATE ../../Library/TeamTalk_DLL)

set (TEAMTALK_LINK_FLAGS Qt6::Widgets Qt6::Xml Qt6::Network TeamTalk5)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_library (CARBON_LIBRARY Carbon)
    list (APPEND TEAMTALK_LINK_FLAGS ${CARBON_LIBRARY})

    set (MACOSX_BUNDLE TRUE)
    set_source_files_properties(images/teamtalk.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(${TEAMTALK_TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST Info.plist)
endif()

target_link_libraries (${TEAMTALK_TARGET} ${TEAMTALK_LINK_FLAGS})
