TEAMTALK_LIB = ../../Library/TeamTalk_DLL
TEAMTALK_INCLUDE = $${TEAMTALK_LIB}

HEADERS    = mainwindow.h preferencesdlg.h uservideowidget.h \
             channelstree.h channeldlg.h userinfodlg.h bannedusersdlg.h \
             useraccountsdlg.h videogridwidget.h uservideodlg.h \
             serverpropertiesdlg.h keycompdlg.h serverlistdlg.h common.h \
             textmessagedlg.h chattextedit.h filesmodel.h filetransferdlg.h \
             filesview.h uservolumedlg.h changestatusdlg.h aboutdlg.h \
             chatlineedit.h serverstatsdlg.h onlineusersdlg.h \
             onlineusersmodel.h chatlineedit.h mediastoragedlg.h \
             desktopgridwidget.h userdesktopwidget.h gridwidget.h \
             userimagewidget.h userdesktopdlg.h desktopsharedlg.h \
             sendtextedit.h streammediafiledlg.h videotextdlg.h \
             desktopaccessdlg.h appinfo.h settings.h generatettfiledlg.h \
             customvideofmtdlg.h license.h bearwarelogindlg.h \
             audiopreprocessordlg.h ttseventsmodel.h \
             statusbardlg.h statusbareventsmodel.h

SOURCES    = main.cpp mainwindow.cpp preferencesdlg.cpp uservideowidget.cpp \
             channelstree.cpp channeldlg.cpp userinfodlg.cpp \
             bannedusersdlg.cpp useraccountsdlg.cpp videogridwidget.cpp \
             uservideodlg.cpp serverpropertiesdlg.cpp keycompdlg.cpp \
             serverlistdlg.cpp common.cpp textmessagedlg.cpp chattextedit.cpp \
             filesmodel.cpp filetransferdlg.cpp filesview.cpp \
             uservolumedlg.cpp changestatusdlg.cpp aboutdlg.cpp \
             chatlineedit.cpp serverstatsdlg.cpp onlineusersdlg.cpp \
             onlineusersmodel.cpp mediastoragedlg.cpp desktopgridwidget.cpp \
             userdesktopwidget.cpp gridwidget.cpp userimagewidget.cpp \
             userdesktopdlg.cpp desktopsharedlg.cpp sendtextedit.cpp \
             streammediafiledlg.cpp videotextdlg.cpp desktopaccessdlg.cpp \
             generatettfiledlg.cpp customvideofmtdlg.cpp \
             bearwarelogindlg.cpp audiopreprocessordlg.cpp ttseventsmodel.cpp \
             statusbardlg.cpp statusbareventsmodel.cpp

FORMS      = mainwindow.ui channel.ui preferences.ui \
             serverlist.ui userinfo.ui bannedusers.ui useraccounts.ui \
             serverproperties.ui uservideo.ui keycomp.ui textmessage.ui \
             filetransfer.ui uservolume.ui changestatus.ui about.ui \
             serverstats.ui onlineusers.ui mediastorage.ui userdesktop.ui \
             desktopshare.ui streammediafile.ui videotext.ui desktopaccess.ui \
             generatettfile.ui customvideofmt.ui bearwarelogindlg.ui \
             audiopreprocessor.ui statusbardlg.ui

RESOURCES += resources.qrc

win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    RC_FILE = mainwindow.rc
    QT += axcontainer
    # Prevent linking to api-ms-win-core-winrt-l1-1-0.dll (doesn't exist on Windows 7)
    QTPLUGIN.audio=qtaudio_windows

    tolk {
        INCLUDEPATH += tolk/src
        LIBS += tolk/bin/\$(PlatformName)/Tolk.lib
        DEFINES += ENABLE_TOLK
    }
}

x11 {
    LIBS += -lX11
}

linux {
    LIBS += -lX11

    greaterThan(QT_MAJOR_VERSION, 4): QT += x11extras
}

mac {
    CONFIG += accessibility
    QTPLUGIN += qtaccessiblewidgets
    # Use 'iconutil' to convert between icns and pngs
    ICON = images/teamtalk.icns
    LIBS += -framework IOKit -framework Carbon
}

# install
target.path = .
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro
sources.path = .
INSTALLS += target sources

INCLUDEPATH += $${TEAMTALK_INCLUDE}

QT += xml network

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia texttospeech

TRANSLATIONS = languages/Bulgarian.ts \
               languages/Chinese_Simplified.ts \
               languages/Chinese_Traditional.ts \
               languages/Czech.ts \
               languages/Danish.ts \
               languages/Dutch.ts \
               languages/English.ts \
               languages/French.ts \
               languages/German.ts \
               languages/Hebrew.ts \
               languages/Hungarian.ts \
               languages/Italian.ts \
               languages/Polish.ts \
               languages/Portuguese_BR.ts \
               languages/Portuguese_EU.ts \
               languages/Russian.ts \
               languages/Slovak.ts \
               languages/Slovenian.ts \
               languages/Spanish.ts \
               languages/Thai.ts \
               languages/Turkish.ts
