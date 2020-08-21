/****************************************************************************
** Meta object code from reading C++ file 'weblogindlg.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/weblogindlg.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'weblogindlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WebLoginDlg_t {
    QByteArrayData data[8];
    char stringdata0[83];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WebLoginDlg_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WebLoginDlg_t qt_meta_stringdata_WebLoginDlg = {
    {
QT_MOC_LITERAL(0, 0, 11), // "WebLoginDlg"
QT_MOC_LITERAL(1, 12, 8), // "navigate"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 3), // "url"
QT_MOC_LITERAL(4, 26, 20), // "slotNavigateComplete"
QT_MOC_LITERAL(5, 47, 10), // "IDispatch*"
QT_MOC_LITERAL(6, 58, 9), // "QVariant&"
QT_MOC_LITERAL(7, 68, 14) // "slotUrlChanged"

    },
    "WebLoginDlg\0navigate\0\0url\0"
    "slotNavigateComplete\0IDispatch*\0"
    "QVariant&\0slotUrlChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WebLoginDlg[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x08 /* Private */,
       4,    2,   32,    2, 0x08 /* Private */,
       7,    1,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 6,    2,    2,
    QMetaType::Void, QMetaType::QUrl,    3,

       0        // eod
};

void WebLoginDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WebLoginDlg *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->navigate((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->slotNavigateComplete((*reinterpret_cast< IDispatch*(*)>(_a[1])),(*reinterpret_cast< QVariant(*)>(_a[2]))); break;
        case 2: _t->slotUrlChanged((*reinterpret_cast< const QUrl(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject WebLoginDlg::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_WebLoginDlg.data,
    qt_meta_data_WebLoginDlg,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *WebLoginDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WebLoginDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WebLoginDlg.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int WebLoginDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
