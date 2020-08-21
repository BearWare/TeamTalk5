/****************************************************************************
** Meta object code from reading C++ file 'textmessagedlg.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/textmessagedlg.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'textmessagedlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TextMessageDlg_t {
    QByteArrayData data[16];
    char stringdata0[184];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TextMessageDlg_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TextMessageDlg_t qt_meta_stringdata_TextMessageDlg = {
    {
QT_MOC_LITERAL(0, 0, 14), // "TextMessageDlg"
QT_MOC_LITERAL(1, 15, 20), // "newMyselfTextMessage"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 11), // "TextMessage"
QT_MOC_LITERAL(4, 49, 7), // "textmsg"
QT_MOC_LITERAL(5, 57, 17), // "closedTextMessage"
QT_MOC_LITERAL(6, 75, 6), // "userid"
QT_MOC_LITERAL(7, 82, 14), // "slotUpdateUser"
QT_MOC_LITERAL(8, 97, 4), // "User"
QT_MOC_LITERAL(9, 102, 4), // "user"
QT_MOC_LITERAL(10, 107, 14), // "slotNewMessage"
QT_MOC_LITERAL(11, 122, 14), // "slotUserLogout"
QT_MOC_LITERAL(12, 137, 10), // "slotCancel"
QT_MOC_LITERAL(13, 148, 11), // "slotSendMsg"
QT_MOC_LITERAL(14, 160, 7), // "txt_msg"
QT_MOC_LITERAL(15, 168, 15) // "slotTextChanged"

    },
    "TextMessageDlg\0newMyselfTextMessage\0"
    "\0TextMessage\0textmsg\0closedTextMessage\0"
    "userid\0slotUpdateUser\0User\0user\0"
    "slotNewMessage\0slotUserLogout\0slotCancel\0"
    "slotSendMsg\0txt_msg\0slotTextChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TextMessageDlg[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       5,    1,   62,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   65,    2, 0x0a /* Public */,
      10,    1,   68,    2, 0x0a /* Public */,
      11,    1,   71,    2, 0x0a /* Public */,
      12,    0,   74,    2, 0x08 /* Private */,
      13,    0,   75,    2, 0x08 /* Private */,
      13,    1,   76,    2, 0x08 /* Private */,
      15,    0,   79,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Int,    6,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void,

       0        // eod
};

void TextMessageDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TextMessageDlg *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->newMyselfTextMessage((*reinterpret_cast< const TextMessage(*)>(_a[1]))); break;
        case 1: _t->closedTextMessage((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotUpdateUser((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 3: _t->slotNewMessage((*reinterpret_cast< const TextMessage(*)>(_a[1]))); break;
        case 4: _t->slotUserLogout((*reinterpret_cast< const User(*)>(_a[1]))); break;
        case 5: _t->slotCancel(); break;
        case 6: _t->slotSendMsg(); break;
        case 7: _t->slotSendMsg((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->slotTextChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TextMessageDlg::*)(const TextMessage & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TextMessageDlg::newMyselfTextMessage)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TextMessageDlg::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TextMessageDlg::closedTextMessage)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TextMessageDlg::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_TextMessageDlg.data,
    qt_meta_data_TextMessageDlg,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TextMessageDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TextMessageDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TextMessageDlg.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int TextMessageDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void TextMessageDlg::newMyselfTextMessage(const TextMessage & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void TextMessageDlg::closedTextMessage(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
