/****************************************************************************
** Meta object code from reading C++ file 'videogridwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/videogridwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'videogridwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VideoGridWidget_t {
    QByteArrayData data[17];
    char stringdata0[210];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VideoGridWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VideoGridWidget_t qt_meta_stringdata_VideoGridWidget = {
    {
QT_MOC_LITERAL(0, 0, 15), // "VideoGridWidget"
QT_MOC_LITERAL(1, 16, 17), // "userVideoSelected"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 8), // "selected"
QT_MOC_LITERAL(4, 44, 14), // "userVideoEnded"
QT_MOC_LITERAL(5, 59, 6), // "userid"
QT_MOC_LITERAL(6, 66, 17), // "videoCountChanged"
QT_MOC_LITERAL(7, 84, 19), // "preferencesModified"
QT_MOC_LITERAL(8, 104, 11), // "slotAddUser"
QT_MOC_LITERAL(9, 116, 14), // "slotRemoveUser"
QT_MOC_LITERAL(10, 131, 9), // "channelid"
QT_MOC_LITERAL(11, 141, 4), // "User"
QT_MOC_LITERAL(12, 146, 4), // "user"
QT_MOC_LITERAL(13, 151, 17), // "slotNewVideoFrame"
QT_MOC_LITERAL(14, 169, 9), // "stream_id"
QT_MOC_LITERAL(15, 179, 22), // "slotVideoWidgetFocused"
QT_MOC_LITERAL(16, 202, 7) // "focused"

    },
    "VideoGridWidget\0userVideoSelected\0\0"
    "selected\0userVideoEnded\0userid\0"
    "videoCountChanged\0preferencesModified\0"
    "slotAddUser\0slotRemoveUser\0channelid\0"
    "User\0user\0slotNewVideoFrame\0stream_id\0"
    "slotVideoWidgetFocused\0focused"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VideoGridWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    1,   57,    2, 0x06 /* Public */,
       6,    1,   60,    2, 0x06 /* Public */,
       7,    0,   63,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   64,    2, 0x0a /* Public */,
       9,    2,   67,    2, 0x0a /* Public */,
      13,    2,   72,    2, 0x0a /* Public */,
      15,    2,   77,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 11,   10,   12,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    5,   14,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    5,   16,

       0        // eod
};

void VideoGridWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VideoGridWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->userVideoSelected((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->userVideoEnded((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->videoCountChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->preferencesModified(); break;
        case 4: _t->slotAddUser((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->slotRemoveUser((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const User(*)>(_a[2]))); break;
        case 6: _t->slotNewVideoFrame((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->slotVideoWidgetFocused((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VideoGridWidget::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoGridWidget::userVideoSelected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VideoGridWidget::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoGridWidget::userVideoEnded)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (VideoGridWidget::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoGridWidget::videoCountChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (VideoGridWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoGridWidget::preferencesModified)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject VideoGridWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<GridWidget::staticMetaObject>(),
    qt_meta_stringdata_VideoGridWidget.data,
    qt_meta_data_VideoGridWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VideoGridWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VideoGridWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VideoGridWidget.stringdata0))
        return static_cast<void*>(this);
    return GridWidget::qt_metacast(_clname);
}

int VideoGridWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = GridWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void VideoGridWidget::userVideoSelected(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void VideoGridWidget::userVideoEnded(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void VideoGridWidget::videoCountChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void VideoGridWidget::preferencesModified()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
