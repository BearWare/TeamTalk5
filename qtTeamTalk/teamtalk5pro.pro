include (mainwindow.pri)

win32:TARGET = TeamTalk5Pro
unix:TARGET = teamtalk5pro
mac:TARGET = TeamTalk5Pro

DEFINES += ENABLE_ENCRYPTION

win32 {
    LIBS += $${TEAMTALK_LIB}/TeamTalk5Pro.lib
}

unix {
    LIBS += -L$${TEAMTALK_LIB} -lTeamTalk5Pro
}
