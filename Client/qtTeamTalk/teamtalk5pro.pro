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

CONFIG(debug, debug|release) {

    MOC_DIR = build/dbg_teamtalk5pro/moc
    RCC_DIR = build/dbg_teamtalk5pro/qrc
    UI_DIR = build/dbg_teamtalk5pro/ui
    OBJECTS_DIR = build/dbg_teamtalk5pro/obj
}

CONFIG(release, debug|release) {

    MOC_DIR = build/rel_teamtalk5pro/moc
    RCC_DIR = build/rel_teamtalk5pro/qrc
    UI_DIR = build/rel_teamtalk5pro/ui
    OBJECTS_DIR = build/rel_teamtalk5pro/obj

}
