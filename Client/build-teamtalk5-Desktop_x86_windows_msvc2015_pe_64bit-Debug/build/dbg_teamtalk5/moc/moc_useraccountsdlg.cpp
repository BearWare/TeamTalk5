/****************************************************************************
** Meta object code from reading C++ file 'useraccountsdlg.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/useraccountsdlg.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'useraccountsdlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_UserAccountsModel_t {
    QByteArrayData data[1];
    char stringdata0[18];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_UserAccountsModel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_UserAccountsModel_t qt_meta_stringdata_UserAccountsModel = {
    {
QT_MOC_LITERAL(0, 0, 17) // "UserAccountsModel"

    },
    "UserAccountsModel"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_UserAccountsModel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void UserAccountsModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject UserAccountsModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractItemModel::staticMetaObject>(),
    qt_meta_stringdata_UserAccountsModel.data,
    qt_meta_data_UserAccountsModel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *UserAccountsModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UserAccountsModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_UserAccountsModel.stringdata0))
        return static_cast<void*>(this);
    return QAbstractItemModel::qt_metacast(_clname);
}

int UserAccountsModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractItemModel::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_UserAccountsDlg_t {
    QByteArrayData data[19];
    char stringdata0[242];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_UserAccountsDlg_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_UserAccountsDlg_t qt_meta_stringdata_UserAccountsDlg = {
    {
QT_MOC_LITERAL(0, 0, 15), // "UserAccountsDlg"
QT_MOC_LITERAL(1, 16, 14), // "slotCmdSuccess"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 5), // "cmdid"
QT_MOC_LITERAL(4, 38, 12), // "slotCmdError"
QT_MOC_LITERAL(5, 51, 5), // "error"
QT_MOC_LITERAL(6, 57, 13), // "slotClearUser"
QT_MOC_LITERAL(7, 71, 11), // "slotAddUser"
QT_MOC_LITERAL(8, 83, 11), // "slotDelUser"
QT_MOC_LITERAL(9, 95, 16), // "slotUserSelected"
QT_MOC_LITERAL(10, 112, 11), // "QModelIndex"
QT_MOC_LITERAL(11, 124, 5), // "index"
QT_MOC_LITERAL(12, 130, 10), // "slotEdited"
QT_MOC_LITERAL(13, 141, 19), // "slotUserTypeChanged"
QT_MOC_LITERAL(14, 161, 18), // "slotCustomCmdLimit"
QT_MOC_LITERAL(15, 180, 16), // "slotAddOpChannel"
QT_MOC_LITERAL(16, 197, 19), // "slotRemoveOpChannel"
QT_MOC_LITERAL(17, 217, 19), // "slotUsernameChanged"
QT_MOC_LITERAL(18, 237, 4) // "text"

    },
    "UserAccountsDlg\0slotCmdSuccess\0\0cmdid\0"
    "slotCmdError\0error\0slotClearUser\0"
    "slotAddUser\0slotDelUser\0slotUserSelected\0"
    "QModelIndex\0index\0slotEdited\0"
    "slotUserTypeChanged\0slotCustomCmdLimit\0"
    "slotAddOpChannel\0slotRemoveOpChannel\0"
    "slotUsernameChanged\0text"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_UserAccountsDlg[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x0a /* Public */,
       4,    2,   77,    2, 0x0a /* Public */,
       6,    0,   82,    2, 0x08 /* Private */,
       7,    0,   83,    2, 0x08 /* Private */,
       8,    0,   84,    2, 0x08 /* Private */,
       9,    1,   85,    2, 0x08 /* Private */,
      12,    1,   88,    2, 0x08 /* Private */,
      13,    0,   91,    2, 0x08 /* Private */,
      14,    1,   92,    2, 0x08 /* Private */,
      15,    0,   95,    2, 0x08 /* Private */,
      16,    0,   96,    2, 0x08 /* Private */,
      17,    1,   97,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    5,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   18,

       0        // eod
};

void UserAccountsDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<UserAccountsDlg *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->slotCmdSuccess((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->slotCmdError((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->slotClearUser(); break;
        case 3: _t->slotAddUser(); break;
        case 4: _t->slotDelUser(); break;
        case 5: _t->slotUserSelected((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 6: _t->slotEdited((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->slotUserTypeChanged(); break;
        case 8: _t->slotCustomCmdLimit((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->slotAddOpChannel(); break;
        case 10: _t->slotRemoveOpChannel(); break;
        case 11: _t->slotUsernameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject UserAccountsDlg::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_UserAccountsDlg.data,
    qt_meta_data_UserAccountsDlg,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *UserAccountsDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UserAccountsDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_UserAccountsDlg.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int UserAccountsDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
