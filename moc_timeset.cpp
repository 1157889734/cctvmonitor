/****************************************************************************
** Meta object code from reading C++ file 'timeset.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "timeset.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'timeset.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_timeset_t {
    QByteArrayData data[13];
    char stringdata0[108];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_timeset_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_timeset_t qt_meta_stringdata_timeset = {
    {
QT_MOC_LITERAL(0, 0, 7), // "timeset"
QT_MOC_LITERAL(1, 8, 14), // "timeSetSendMsg"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 4), // "year"
QT_MOC_LITERAL(4, 29, 5), // "month"
QT_MOC_LITERAL(5, 35, 3), // "day"
QT_MOC_LITERAL(6, 39, 4), // "hour"
QT_MOC_LITERAL(7, 44, 3), // "min"
QT_MOC_LITERAL(8, 48, 3), // "sec"
QT_MOC_LITERAL(9, 52, 9), // "cancleMsg"
QT_MOC_LITERAL(10, 62, 17), // "cancleButtonClick"
QT_MOC_LITERAL(11, 80, 13), // "okButtonClick"
QT_MOC_LITERAL(12, 94, 13) // "stButtonClick"

    },
    "timeset\0timeSetSendMsg\0\0year\0month\0"
    "day\0hour\0min\0sec\0cancleMsg\0cancleButtonClick\0"
    "okButtonClick\0stButtonClick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_timeset[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    6,   39,    2, 0x06 /* Public */,
       9,    0,   52,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   53,    2, 0x0a /* Public */,
      11,    0,   54,    2, 0x0a /* Public */,
      12,    0,   55,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString,    3,    4,    5,    6,    7,    8,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void timeset::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<timeset *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->timeSetSendMsg((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5])),(*reinterpret_cast< QString(*)>(_a[6]))); break;
        case 1: _t->cancleMsg(); break;
        case 2: _t->cancleButtonClick(); break;
        case 3: _t->okButtonClick(); break;
        case 4: _t->stButtonClick(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (timeset::*)(QString , QString , QString , QString , QString , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&timeset::timeSetSendMsg)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (timeset::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&timeset::cancleMsg)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject timeset::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_timeset.data,
    qt_meta_data_timeset,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *timeset::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *timeset::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_timeset.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int timeset::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void timeset::timeSetSendMsg(QString _t1, QString _t2, QString _t3, QString _t4, QString _t5, QString _t6)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void timeset::cancleMsg()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
