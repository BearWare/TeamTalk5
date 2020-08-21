/****************************************************************************
** Meta object code from reading C++ file 'streammediafiledlg.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qtTeamTalk/streammediafiledlg.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'streammediafiledlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_StreamMediaFileDlg_t {
    QByteArrayData data[15];
    char stringdata0[235];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_StreamMediaFileDlg_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_StreamMediaFileDlg_t qt_meta_stringdata_StreamMediaFileDlg = {
    {
QT_MOC_LITERAL(0, 0, 18), // "StreamMediaFileDlg"
QT_MOC_LITERAL(1, 19, 12), // "slotAccepted"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 14), // "slotSelectFile"
QT_MOC_LITERAL(4, 48, 17), // "slotSelectionFile"
QT_MOC_LITERAL(5, 66, 19), // "showMediaFormatInfo"
QT_MOC_LITERAL(6, 86, 14), // "updateControls"
QT_MOC_LITERAL(7, 101, 14), // "updateProgress"
QT_MOC_LITERAL(8, 116, 7), // "elapsed"
QT_MOC_LITERAL(9, 124, 8), // "setvalue"
QT_MOC_LITERAL(10, 133, 17), // "slotPlayMediaFile"
QT_MOC_LITERAL(11, 151, 17), // "slotStopMediaFile"
QT_MOC_LITERAL(12, 169, 20), // "slotChangePlayOffset"
QT_MOC_LITERAL(13, 190, 22), // "slotChangePreprocessor"
QT_MOC_LITERAL(14, 213, 21) // "slotSetupPreprocessor"

    },
    "StreamMediaFileDlg\0slotAccepted\0\0"
    "slotSelectFile\0slotSelectionFile\0"
    "showMediaFormatInfo\0updateControls\0"
    "updateProgress\0elapsed\0setvalue\0"
    "slotPlayMediaFile\0slotStopMediaFile\0"
    "slotChangePlayOffset\0slotChangePreprocessor\0"
    "slotSetupPreprocessor"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_StreamMediaFileDlg[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x08 /* Private */,
       3,    0,   70,    2, 0x08 /* Private */,
       4,    1,   71,    2, 0x08 /* Private */,
       5,    0,   74,    2, 0x08 /* Private */,
       6,    0,   75,    2, 0x08 /* Private */,
       7,    2,   76,    2, 0x08 /* Private */,
      10,    0,   81,    2, 0x08 /* Private */,
      11,    0,   82,    2, 0x08 /* Private */,
      12,    1,   83,    2, 0x08 /* Private */,
      13,    1,   86,    2, 0x08 /* Private */,
      14,    1,   89,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt, QMetaType::Bool,    8,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Bool,    2,

       0        // eod
};

void StreamMediaFileDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<StreamMediaFileDlg *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->slotAccepted(); break;
        case 1: _t->slotSelectFile(); break;
        case 2: _t->slotSelectionFile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->showMediaFormatInfo(); break;
        case 4: _t->updateControls(); break;
        case 5: _t->updateProgress((*reinterpret_cast< quint32(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 6: _t->slotPlayMediaFile(); break;
        case 7: _t->slotStopMediaFile(); break;
        case 8: _t->slotChangePlayOffset((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->slotChangePreprocessor((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->slotSetupPreprocessor((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject StreamMediaFileDlg::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_StreamMediaFileDlg.data,
    qt_meta_data_StreamMediaFileDlg,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *StreamMediaFileDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StreamMediaFileDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_StreamMediaFileDlg.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int StreamMediaFileDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
