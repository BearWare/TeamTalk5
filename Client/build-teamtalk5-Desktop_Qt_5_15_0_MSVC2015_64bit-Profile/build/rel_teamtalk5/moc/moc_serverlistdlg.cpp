/****************************************************************************
** Meta object code from reading C++ file 'serverlistdlg.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/serverlistdlg.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'serverlistdlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ServerListDlg_t {
    QByteArrayData data[23];
    char stringdata0[330];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ServerListDlg_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ServerListDlg_t qt_meta_stringdata_ServerListDlg = {
    {
QT_MOC_LITERAL(0, 0, 13), // "ServerListDlg"
QT_MOC_LITERAL(1, 14, 12), // "slotShowHost"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 5), // "index"
QT_MOC_LITERAL(4, 34, 14), // "slotShowServer"
QT_MOC_LITERAL(5, 49, 16), // "slotAddUpdServer"
QT_MOC_LITERAL(6, 66, 16), // "slotDeleteServer"
QT_MOC_LITERAL(7, 83, 22), // "slotClearServerClicked"
QT_MOC_LITERAL(8, 106, 11), // "slotConnect"
QT_MOC_LITERAL(9, 118, 18), // "slotServerSelected"
QT_MOC_LITERAL(10, 137, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(11, 154, 4), // "item"
QT_MOC_LITERAL(12, 159, 17), // "slotDoubleClicked"
QT_MOC_LITERAL(13, 177, 15), // "slotFreeServers"
QT_MOC_LITERAL(14, 193, 7), // "checked"
QT_MOC_LITERAL(15, 201, 21), // "slotFreeServerRequest"
QT_MOC_LITERAL(16, 223, 14), // "QNetworkReply*"
QT_MOC_LITERAL(17, 238, 5), // "reply"
QT_MOC_LITERAL(18, 244, 16), // "slotGenerateFile"
QT_MOC_LITERAL(19, 261, 20), // "slotDeleteLatestHost"
QT_MOC_LITERAL(20, 282, 20), // "slotSaveEntryChanged"
QT_MOC_LITERAL(21, 303, 4), // "text"
QT_MOC_LITERAL(22, 308, 21) // "slotGenerateEntryName"

    },
    "ServerListDlg\0slotShowHost\0\0index\0"
    "slotShowServer\0slotAddUpdServer\0"
    "slotDeleteServer\0slotClearServerClicked\0"
    "slotConnect\0slotServerSelected\0"
    "QListWidgetItem*\0item\0slotDoubleClicked\0"
    "slotFreeServers\0checked\0slotFreeServerRequest\0"
    "QNetworkReply*\0reply\0slotGenerateFile\0"
    "slotDeleteLatestHost\0slotSaveEntryChanged\0"
    "text\0slotGenerateEntryName"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ServerListDlg[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x08 /* Private */,
       4,    1,   87,    2, 0x08 /* Private */,
       5,    0,   90,    2, 0x08 /* Private */,
       6,    0,   91,    2, 0x08 /* Private */,
       7,    0,   92,    2, 0x08 /* Private */,
       8,    0,   93,    2, 0x08 /* Private */,
       9,    1,   94,    2, 0x08 /* Private */,
      12,    1,   97,    2, 0x08 /* Private */,
      13,    1,  100,    2, 0x08 /* Private */,
      15,    1,  103,    2, 0x08 /* Private */,
      18,    0,  106,    2, 0x08 /* Private */,
      19,    0,  107,    2, 0x08 /* Private */,
      20,    1,  108,    2, 0x08 /* Private */,
      22,    1,  111,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, 0x80000000 | 10,    2,
    QMetaType::Void, QMetaType::Bool,   14,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   21,
    QMetaType::Void, QMetaType::QString,    2,

       0        // eod
};

void ServerListDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ServerListDlg *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->slotShowHost((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->slotShowServer((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotAddUpdServer(); break;
        case 3: _t->slotDeleteServer(); break;
        case 4: _t->slotClearServerClicked(); break;
        case 5: _t->slotConnect(); break;
        case 6: _t->slotServerSelected((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 7: _t->slotDoubleClicked((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 8: _t->slotFreeServers((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->slotFreeServerRequest((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 10: _t->slotGenerateFile(); break;
        case 11: _t->slotDeleteLatestHost(); break;
        case 12: _t->slotSaveEntryChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->slotGenerateEntryName((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ServerListDlg::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_ServerListDlg.data,
    qt_meta_data_ServerListDlg,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ServerListDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ServerListDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ServerListDlg.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int ServerListDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
