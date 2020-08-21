/****************************************************************************
** Meta object code from reading C++ file 'userdesktopwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/userdesktopwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'userdesktopwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_UserDesktopWidget_t {
    QByteArrayData data[15];
    char stringdata0[175];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_UserDesktopWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_UserDesktopWidget_t qt_meta_stringdata_UserDesktopWidget = {
    {
QT_MOC_LITERAL(0, 0, 17), // "UserDesktopWidget"
QT_MOC_LITERAL(1, 18, 22), // "userDesktopWindowEnded"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 6), // "userid"
QT_MOC_LITERAL(4, 49, 17), // "slotDesktopUpdate"
QT_MOC_LITERAL(5, 67, 9), // "sessionid"
QT_MOC_LITERAL(6, 77, 23), // "slotDesktopCursorUpdate"
QT_MOC_LITERAL(7, 101, 10), // "src_userid"
QT_MOC_LITERAL(8, 112, 12), // "DesktopInput"
QT_MOC_LITERAL(9, 125, 5), // "input"
QT_MOC_LITERAL(10, 131, 15), // "slotUserUpdated"
QT_MOC_LITERAL(11, 147, 4), // "User"
QT_MOC_LITERAL(12, 152, 4), // "user"
QT_MOC_LITERAL(13, 157, 15), // "slotContextMenu"
QT_MOC_LITERAL(14, 173, 1) // "p"

    },
    "UserDesktopWidget\0userDesktopWindowEnded\0"
    "\0userid\0slotDesktopUpdate\0sessionid\0"
    "slotDesktopCursorUpdate\0src_userid\0"
    "DesktopInput\0input\0slotUserUpdated\0"
    "User\0user\0slotContextMenu\0p"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_UserDesktopWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    2,   42,    2, 0x0a /* Public */,
       6,    2,   47,    2, 0x0a /* Public */,
      10,    1,   52,    2, 0x0a /* Public */,
      13,    1,   55,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    5,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 8,    7,    9,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void, QMetaType::QPoint,   14,

       0        // eod
};

void UserDesktopWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<UserDesktopWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->userDesktopWindowEnded((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->slotDesktopUpdate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->slotDesktopCursorUpdate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const DesktopInput(*)>(_a[2]))); break;
        case 3: _t->slotUserUpdated((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 4: _t->slotContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (UserDesktopWidget::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserDesktopWidget::userDesktopWindowEnded)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject UserDesktopWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<UserImageWidget::staticMetaObject>(),
    qt_meta_stringdata_UserDesktopWidget.data,
    qt_meta_data_UserDesktopWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *UserDesktopWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UserDesktopWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_UserDesktopWidget.stringdata0))
        return static_cast<void*>(this);
    return UserImageWidget::qt_metacast(_clname);
}

int UserDesktopWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = UserImageWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void UserDesktopWidget::userDesktopWindowEnded(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
