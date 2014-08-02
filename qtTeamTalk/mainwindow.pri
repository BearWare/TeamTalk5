HEADERS    = mainwindow.h preferencesdlg.h uservideowidget.h \
             channelstree.h channeldlg.h userinfodlg.h bannedusersdlg.h \
             useraccountsdlg.h videogridwidget.h uservideodlg.h \
             serverpropertiesdlg.h keycompdlg.h serverlistdlg.h common.h \
             textmessagedlg.h chattextedit.h filesmodel.h filetransferdlg.h \
             filesview.h uservolumedlg.h changestatusdlg.h aboutdlg.h \
             chatlineedit.h serverstatsdlg.h onlineusersdlg.h \
             onlineusersmodel.h chatlineedit.h audiostoragedlg.h \
             desktopgridwidget.h userdesktopwidget.h gridwidget.h \
             userimagewidget.h userdesktopdlg.h desktopsharedlg.h \
             sendtextedit.h streammediafiledlg.h videotextdlg.h \
             desktopaccessdlg.h appinfo.h settings.h

SOURCES    = main.cpp mainwindow.cpp preferencesdlg.cpp uservideowidget.cpp \
             channelstree.cpp channeldlg.cpp userinfodlg.cpp \
             bannedusersdlg.cpp useraccountsdlg.cpp videogridwidget.cpp \
             uservideodlg.cpp serverpropertiesdlg.cpp keycompdlg.cpp \
             serverlistdlg.cpp common.cpp textmessagedlg.cpp chattextedit.cpp \
             filesmodel.cpp filetransferdlg.cpp filesview.cpp \
             uservolumedlg.cpp changestatusdlg.cpp aboutdlg.cpp \
             chatlineedit.cpp serverstatsdlg.cpp onlineusersdlg.cpp \
             onlineusersmodel.cpp audiostoragedlg.cpp desktopgridwidget.cpp \
             userdesktopwidget.cpp gridwidget.cpp userimagewidget.cpp \
             userdesktopdlg.cpp desktopsharedlg.cpp sendtextedit.cpp \
             streammediafiledlg.cpp videotextdlg.cpp desktopaccessdlg.cpp

FORMS      = mainwindow.ui channel.ui preferences.ui \
             serverlist.ui userinfo.ui bannedusers.ui useraccounts.ui \
             serverproperties.ui uservideo.ui keycomp.ui textmessage.ui \
             filetransfer.ui uservolume.ui changestatus.ui about.ui \
             serverstats.ui onlineusers.ui audiostorage.ui userdesktop.ui \
             desktopshare.ui streammediafile.ui videotext.ui desktopaccess.ui

RESOURCES += resources.qrc

win32 {
      DEFINES += _CRT_SECURE_NO_WARNINGS
}

x11 {
     LIBS += -lX11
}

mac {
    ICON = images/teamtalk.icns
    LIBS += -framework IOKit -framework Carbon
}

# install
target.path = .
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro
sources.path = .
INSTALLS += target sources

INCLUDEPATH += ../TeamTalk_DLL

QT += xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

TRANSLATIONS = languages/Chinese_Simplified.ts languages/Chinese_Traditional.ts \
               languages/Czech.ts languages/Danish.ts \
               languages/Dutch.ts languages/English.ts \
               languages/French.ts languages/German.ts \
               languages/Hebrew.ts languages/Italian.ts \
               languages/Polish.ts languages/Portuguese_BR.ts \
               languages/Portuguese_EU.ts languages/Russian.ts \
               languages/Slovak.ts languages/Slovenian.ts \
               languages/Spanish.ts languages/Thai.ts \
               languages/Turkish.ts

