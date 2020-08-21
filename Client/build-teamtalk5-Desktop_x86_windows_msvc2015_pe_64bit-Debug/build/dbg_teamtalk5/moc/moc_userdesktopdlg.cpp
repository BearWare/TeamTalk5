/****************************************************************************
** Meta object code from reading C++ file 'userdesktopdlg.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/userdesktopdlg.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'userdesktopdlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_UserDesktopDlg_t {
    QByteArrayData data[13];
    char stringdata0[176];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_UserDesktopDlg_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_UserDesktopDlg_t qt_meta_stringdata_UserDesktopDlg = {
    {
QT_MOC_LITERAL(0, 0, 14), // "UserDesktopDlg"
QT_MOC_LITERAL(1, 15, 21), // "userDesktopDlgClosing"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 6), // "userid"
QT_MOC_LITERAL(4, 45, 23), // "userDesktopWindowUpdate"
QT_MOC_LITERAL(5, 69, 9), // "sessionid"
QT_MOC_LITERAL(6, 79, 22), // "userDesktopWindowEnded"
QT_MOC_LITERAL(7, 102, 23), // "userDesktopCursorUpdate"
QT_MOC_LITERAL(8, 126, 12), // "DesktopInput"
QT_MOC_LITERAL(9, 139, 11), // "userUpdated"
QT_MOC_LITERAL(10, 151, 4), // "User"
QT_MOC_LITERAL(11, 156, 14), // "slotUserUpdate"
QT_MOC_LITERAL(12, 171, 4) // "user"

    },
    "UserDesktopDlg\0userDesktopDlgClosing\0"
    "\0userid\0userDesktopWindowUpdate\0"
    "sessionid\0userDesktopWindowEnded\0"
    "userDesktopCursorUpdate\0DesktopInput\0"
    "userUpdated\0User\0slotUserUpdate\0user"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_UserDesktopDlg[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       4,    2,   47,    2, 0x06 /* Public */,
       6,    1,   52,    2, 0x06 /* Public */,
       7,    2,   55,    2, 0x06 /* Public */,
       9,    1,   60,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    1,   63,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    5,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 8,    2,    2,
    QMetaType::Void, 0x80000000 | 10,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 10,   12,

       0        // eod
};

void UserDesktopDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<UserDesktopDlg *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->userDesktopDlgClosing((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->userDesktopWindowUpdate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->userDesktopWindowEnded((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->userDesktopCursorUpdate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const DesktopInput(*)>(_a[2]))); break;
        case 4: _t->userUpdated((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 5: _t->slotUserUpdate((*reinterpret_cast< const User(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (UserDesktopDlg::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserDesktopDlg::userDesktopDlgClosing)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (UserDesktopDlg::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserDesktopDlg::userDesktopWindowUpdate)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (UserDesktopDlg::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserDesktopDlg::userDesktopWindowEnded)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (UserDesktopDlg::*)(int , const DesktopInput & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserDesktopDlg::userDesktopCursorUpdate)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (UserDesktopDlg::*)(const User & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserDesktopDlg::userUpdated)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject UserDesktopDlg::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_UserDesktopDlg.data,
    qt_meta_data_UserDesktopDlg,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *UserDesktopDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UserDesktopDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_UserDesktopDlg.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int UserDesktopDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void UserDesktopDlg::userDesktopDlgClosing(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void UserDesktopDlg::userDesktopWindowUpdate(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void UserDesktopDlg::userDesktopWindowEnded(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void UserDesktopDlg::userDesktopCursorUpdate(int _t1, const DesktopInput & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void UserDesktopDlg::userUpdated(const User & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
