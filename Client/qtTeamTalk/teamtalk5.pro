include (mainwindow.pri)

win32:TARGET = TeamTalk5
unix:TARGET = teamtalk5
mac:TARGET = TeamTalk5

QMAKE_INFO_PLIST = Info.plist

win32 {
    LIBS += $${TEAMTALK_LIB}/TeamTalk5.lib
}
unix {
    LIBS += -L$${TEAMTALK_LIB} -lTeamTalk5
}

CONFIG(debug, debug|release) {

    MOC_DIR = build/dbg_teamtalk5/moc
    RCC_DIR = build/dbg_teamtalk5/qrc
    UI_DIR = build/dbg_teamtalk5/ui
    OBJECTS_DIR = build/dbg_teamtalk5/obj
}

CONFIG(release, debug|release) {

    MOC_DIR = build/rel_teamtalk5/moc
    RCC_DIR = build/rel_teamtalk5/qrc
    UI_DIR = build/rel_teamtalk5/ui
    OBJECTS_DIR = build/rel_teamtalk5/obj

    mac {
        LIBS += -sectcreate __TEXT __info_plist $${TARGET}.app/Contents/Info.plist
    }
}

@
QMAKE_POST_LINK = lrelease teamtalk5.pro
@

