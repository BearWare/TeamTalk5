include (mainwindow.pri)

win32:TARGET = TeamTalk5
unix:TARGET = teamtalk5
mac:TARGET = TeamTalk5

win32 {
    LIBS += ../TeamTalk_DLL/TeamTalk5.lib
}
unix {
    LIBS += -L../TeamTalk_DLL -lTeamTalk5
}
