/****************************************************************************
** Meta object code from reading C++ file 'onlineusersdlg.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/onlineusersdlg.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'onlineusersdlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OnlineUsersDlg_t {
    QByteArrayData data[22];
    char stringdata0[258];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OnlineUsersDlg_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OnlineUsersDlg_t qt_meta_stringdata_OnlineUsersDlg = {
    {
QT_MOC_LITERAL(0, 0, 14), // "OnlineUsersDlg"
QT_MOC_LITERAL(1, 15, 19), // "viewUserInformation"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 6), // "userid"
QT_MOC_LITERAL(4, 43, 15), // "sendUserMessage"
QT_MOC_LITERAL(5, 59, 8), // "muteUser"
QT_MOC_LITERAL(6, 68, 4), // "mute"
QT_MOC_LITERAL(7, 73, 16), // "changeUserVolume"
QT_MOC_LITERAL(8, 90, 6), // "opUser"
QT_MOC_LITERAL(9, 97, 6), // "chanid"
QT_MOC_LITERAL(10, 104, 8), // "kickUser"
QT_MOC_LITERAL(11, 113, 11), // "kickbanUser"
QT_MOC_LITERAL(12, 125, 16), // "streamfileToUser"
QT_MOC_LITERAL(13, 142, 16), // "slotUserLoggedIn"
QT_MOC_LITERAL(14, 159, 4), // "User"
QT_MOC_LITERAL(15, 164, 4), // "user"
QT_MOC_LITERAL(16, 169, 17), // "slotUserLoggedOut"
QT_MOC_LITERAL(17, 187, 14), // "slotUserUpdate"
QT_MOC_LITERAL(18, 202, 12), // "slotUserJoin"
QT_MOC_LITERAL(19, 215, 9), // "channelid"
QT_MOC_LITERAL(20, 225, 12), // "slotUserLeft"
QT_MOC_LITERAL(21, 238, 19) // "slotTreeContextMenu"

    },
    "OnlineUsersDlg\0viewUserInformation\0\0"
    "userid\0sendUserMessage\0muteUser\0mute\0"
    "changeUserVolume\0opUser\0chanid\0kickUser\0"
    "kickbanUser\0streamfileToUser\0"
    "slotUserLoggedIn\0User\0user\0slotUserLoggedOut\0"
    "slotUserUpdate\0slotUserJoin\0channelid\0"
    "slotUserLeft\0slotTreeContextMenu"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OnlineUsersDlg[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x06 /* Public */,
       4,    1,   87,    2, 0x06 /* Public */,
       5,    2,   90,    2, 0x06 /* Public */,
       7,    1,   95,    2, 0x06 /* Public */,
       8,    2,   98,    2, 0x06 /* Public */,
      10,    2,  103,    2, 0x06 /* Public */,
      11,    2,  108,    2, 0x06 /* Public */,
      12,    1,  113,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    1,  116,    2, 0x0a /* Public */,
      16,    1,  119,    2, 0x0a /* Public */,
      17,    1,  122,    2, 0x0a /* Public */,
      18,    2,  125,    2, 0x0a /* Public */,
      20,    2,  130,    2, 0x0a /* Public */,
      21,    1,  135,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    3,    6,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    9,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    9,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    9,
    QMetaType::Void, QMetaType::Int,    3,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 14,   19,   15,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 14,   19,   15,
    QMetaType::Void, QMetaType::QPoint,    2,

       0        // eod
};

void OnlineUsersDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<OnlineUsersDlg *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->viewUserInformation((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sendUserMessage((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->muteUser((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->changeUserVolume((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->opUser((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->kickUser((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->kickbanUser((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->streamfileToUser((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->slotUserLoggedIn((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 9: _t->slotUserLoggedOut((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 10: _t->slotUserUpdate((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 11: _t->slotUserJoin((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const User(*)>(_a[2]))); break;
        case 12: _t->slotUserLeft((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const User(*)>(_a[2]))); break;
        case 13: _t->slotTreeContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (OnlineUsersDlg::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&OnlineUsersDlg::viewUserInformation)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (OnlineUsersDlg::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&OnlineUsersDlg::sendUserMessage)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (OnlineUsersDlg::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&OnlineUsersDlg::muteUser)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (OnlineUsersDlg::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&OnlineUsersDlg::changeUserVolume)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (OnlineUsersDlg::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&OnlineUsersDlg::opUser)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (OnlineUsersDlg::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&OnlineUsersDlg::kickUser)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (OnlineUsersDlg::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&OnlineUsersDlg::kickbanUser)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (OnlineUsersDlg::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&OnlineUsersDlg::streamfileToUser)) {
                *result = 7;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject OnlineUsersDlg::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_OnlineUsersDlg.data,
    qt_meta_data_OnlineUsersDlg,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *OnlineUsersDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OnlineUsersDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_OnlineUsersDlg.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int OnlineUsersDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void OnlineUsersDlg::viewUserInformation(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void OnlineUsersDlg::sendUserMessage(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void OnlineUsersDlg::muteUser(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void OnlineUsersDlg::changeUserVolume(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void OnlineUsersDlg::opUser(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void OnlineUsersDlg::kickUser(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void OnlineUsersDlg::kickbanUser(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void OnlineUsersDlg::streamfileToUser(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
