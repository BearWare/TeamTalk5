include (mainwindow.pri)

win32:TARGET = TeamTalk5
unix:TARGET = teamtalk5
mac:TARGET = TeamTalk5

win32 {
    LIBS += $${TEAMTALK_LIB}/TeamTalk5.lib
}
unix {
    LIBS += -L$${TEAMTALK_LIB} -lTeamTalk5
}
