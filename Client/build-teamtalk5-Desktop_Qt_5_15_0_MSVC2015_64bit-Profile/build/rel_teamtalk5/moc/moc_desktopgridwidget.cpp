/****************************************************************************
** Meta object code from reading C++ file 'desktopgridwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/desktopgridwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'desktopgridwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DesktopGridWidget_t {
    QByteArrayData data[18];
    char stringdata0[251];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DesktopGridWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DesktopGridWidget_t qt_meta_stringdata_DesktopGridWidget = {
    {
QT_MOC_LITERAL(0, 0, 17), // "DesktopGridWidget"
QT_MOC_LITERAL(1, 18, 23), // "userDesktopWindowUpdate"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 22), // "userDesktopWindowEnded"
QT_MOC_LITERAL(4, 66, 19), // "userDesktopSelected"
QT_MOC_LITERAL(5, 86, 8), // "selected"
QT_MOC_LITERAL(6, 95, 23), // "userDesktopCursorUpdate"
QT_MOC_LITERAL(7, 119, 12), // "DesktopInput"
QT_MOC_LITERAL(8, 132, 11), // "userUpdated"
QT_MOC_LITERAL(9, 144, 4), // "User"
QT_MOC_LITERAL(10, 149, 19), // "desktopCountChanged"
QT_MOC_LITERAL(11, 169, 11), // "slotAddUser"
QT_MOC_LITERAL(12, 181, 6), // "userid"
QT_MOC_LITERAL(13, 188, 14), // "slotRemoveUser"
QT_MOC_LITERAL(14, 203, 9), // "channelid"
QT_MOC_LITERAL(15, 213, 4), // "user"
QT_MOC_LITERAL(16, 218, 24), // "slotDesktopWidgetFocused"
QT_MOC_LITERAL(17, 243, 7) // "focused"

    },
    "DesktopGridWidget\0userDesktopWindowUpdate\0"
    "\0userDesktopWindowEnded\0userDesktopSelected\0"
    "selected\0userDesktopCursorUpdate\0"
    "DesktopInput\0userUpdated\0User\0"
    "desktopCountChanged\0slotAddUser\0userid\0"
    "slotRemoveUser\0channelid\0user\0"
    "slotDesktopWidgetFocused\0focused"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DesktopGridWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   59,    2, 0x06 /* Public */,
       3,    1,   64,    2, 0x06 /* Public */,
       4,    1,   67,    2, 0x06 /* Public */,
       6,    2,   70,    2, 0x06 /* Public */,
       8,    1,   75,    2, 0x06 /* Public */,
      10,    1,   78,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    1,   81,    2, 0x0a /* Public */,
      13,    2,   84,    2, 0x0a /* Public */,
      16,    2,   89,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    2,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 7,    2,    2,
    QMetaType::Void, 0x80000000 | 9,    2,
    QMetaType::Void, QMetaType::Int,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 9,   14,   15,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,   12,   17,

       0        // eod
};

void DesktopGridWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DesktopGridWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->userDesktopWindowUpdate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->userDesktopWindowEnded((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->userDesktopSelected((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->userDesktopCursorUpdate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const DesktopInput(*)>(_a[2]))); break;
        case 4: _t->userUpdated((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 5: _t->desktopCountChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotAddUser((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->slotRemoveUser((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const User(*)>(_a[2]))); break;
        case 8: _t->slotDesktopWidgetFocused((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DesktopGridWidget::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DesktopGridWidget::userDesktopWindowUpdate)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DesktopGridWidget::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DesktopGridWidget::userDesktopWindowEnded)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DesktopGridWidget::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DesktopGridWidget::userDesktopSelected)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DesktopGridWidget::*)(int , const DesktopInput & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DesktopGridWidget::userDesktopCursorUpdate)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DesktopGridWidget::*)(const User & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DesktopGridWidget::userUpdated)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DesktopGridWidget::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DesktopGridWidget::desktopCountChanged)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DesktopGridWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<GridWidget::staticMetaObject>(),
    qt_meta_stringdata_DesktopGridWidget.data,
    qt_meta_data_DesktopGridWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DesktopGridWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DesktopGridWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DesktopGridWidget.stringdata0))
        return static_cast<void*>(this);
    return GridWidget::qt_metacast(_clname);
}

int DesktopGridWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = GridWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void DesktopGridWidget::userDesktopWindowUpdate(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DesktopGridWidget::userDesktopWindowEnded(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DesktopGridWidget::userDesktopSelected(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DesktopGridWidget::userDesktopCursorUpdate(int _t1, const DesktopInput & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DesktopGridWidget::userUpdated(const User & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void DesktopGridWidget::desktopCountChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
