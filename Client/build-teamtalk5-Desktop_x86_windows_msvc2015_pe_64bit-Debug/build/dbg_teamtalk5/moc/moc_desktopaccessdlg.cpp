/****************************************************************************
** Meta object code from reading C++ file 'desktopaccessdlg.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/desktopaccessdlg.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'desktopaccessdlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DesktopAccessDlg_t {
    QByteArrayData data[13];
    char stringdata0[165];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DesktopAccessDlg_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DesktopAccessDlg_t qt_meta_stringdata_DesktopAccessDlg = {
    {
QT_MOC_LITERAL(0, 0, 16), // "DesktopAccessDlg"
QT_MOC_LITERAL(1, 17, 6), // "accept"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 12), // "slotDelEntry"
QT_MOC_LITERAL(4, 38, 14), // "slotClearEntry"
QT_MOC_LITERAL(5, 53, 12), // "slotAddEntry"
QT_MOC_LITERAL(6, 66, 14), // "slotAddChannel"
QT_MOC_LITERAL(7, 81, 14), // "slotDelChannel"
QT_MOC_LITERAL(8, 96, 15), // "slotAddUsername"
QT_MOC_LITERAL(9, 112, 15), // "slotDelUsername"
QT_MOC_LITERAL(10, 128, 18), // "slotServerSelected"
QT_MOC_LITERAL(11, 147, 11), // "QModelIndex"
QT_MOC_LITERAL(12, 159, 5) // "index"

    },
    "DesktopAccessDlg\0accept\0\0slotDelEntry\0"
    "slotClearEntry\0slotAddEntry\0slotAddChannel\0"
    "slotDelChannel\0slotAddUsername\0"
    "slotDelUsername\0slotServerSelected\0"
    "QModelIndex\0index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DesktopAccessDlg[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x0a /* Public */,
       3,    0,   60,    2, 0x08 /* Private */,
       4,    0,   61,    2, 0x08 /* Private */,
       5,    0,   62,    2, 0x08 /* Private */,
       6,    0,   63,    2, 0x08 /* Private */,
       7,    0,   64,    2, 0x08 /* Private */,
       8,    0,   65,    2, 0x08 /* Private */,
       9,    0,   66,    2, 0x08 /* Private */,
      10,    1,   67,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 11,   12,

       0        // eod
};

void DesktopAccessDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DesktopAccessDlg *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->accept(); break;
        case 1: _t->slotDelEntry(); break;
        case 2: _t->slotClearEntry(); break;
        case 3: _t->slotAddEntry(); break;
        case 4: _t->slotAddChannel(); break;
        case 5: _t->slotDelChannel(); break;
        case 6: _t->slotAddUsername(); break;
        case 7: _t->slotDelUsername(); break;
        case 8: _t->slotServerSelected((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DesktopAccessDlg::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_DesktopAccessDlg.data,
    qt_meta_data_DesktopAccessDlg,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DesktopAccessDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DesktopAccessDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DesktopAccessDlg.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int DesktopAccessDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
