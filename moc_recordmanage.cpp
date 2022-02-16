/****************************************************************************
** Meta object code from reading C++ file 'recordmanage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "recordmanage.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'recordmanage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_recordManage_t {
    QByteArrayData data[17];
    char stringdata0[209];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_recordManage_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_recordManage_t qt_meta_stringdata_recordManage = {
    {
QT_MOC_LITERAL(0, 0, 12), // "recordManage"
QT_MOC_LITERAL(1, 13, 14), // "hideRecSysPage"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 16), // "hideRecPageSlots"
QT_MOC_LITERAL(4, 46, 26), // "openStartTimeSetWidgetSlot"
QT_MOC_LITERAL(5, 73, 25), // "openStopTimeSetWidgetSlot"
QT_MOC_LITERAL(6, 99, 14), // "timeSetRecvMsg"
QT_MOC_LITERAL(7, 114, 4), // "year"
QT_MOC_LITERAL(8, 119, 5), // "month"
QT_MOC_LITERAL(9, 125, 3), // "day"
QT_MOC_LITERAL(10, 129, 4), // "hour"
QT_MOC_LITERAL(11, 134, 3), // "min"
QT_MOC_LITERAL(12, 138, 3), // "sec"
QT_MOC_LITERAL(13, 142, 15), // "carNoChangeSlot"
QT_MOC_LITERAL(14, 158, 16), // "SearchBtnClicked"
QT_MOC_LITERAL(15, 175, 18), // "recordQueryEndSlot"
QT_MOC_LITERAL(16, 194, 14) // "DownBtnClicked"

    },
    "recordManage\0hideRecSysPage\0\0"
    "hideRecPageSlots\0openStartTimeSetWidgetSlot\0"
    "openStopTimeSetWidgetSlot\0timeSetRecvMsg\0"
    "year\0month\0day\0hour\0min\0sec\0carNoChangeSlot\0"
    "SearchBtnClicked\0recordQueryEndSlot\0"
    "DownBtnClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_recordManage[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   60,    2, 0x0a /* Public */,
       4,    0,   61,    2, 0x0a /* Public */,
       5,    0,   62,    2, 0x0a /* Public */,
       6,    6,   63,    2, 0x0a /* Public */,
      13,    0,   76,    2, 0x0a /* Public */,
      14,    0,   77,    2, 0x0a /* Public */,
      15,    0,   78,    2, 0x0a /* Public */,
      16,    0,   79,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString,    7,    8,    9,   10,   11,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void recordManage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<recordManage *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->hideRecSysPage(); break;
        case 1: _t->hideRecPageSlots(); break;
        case 2: _t->openStartTimeSetWidgetSlot(); break;
        case 3: _t->openStopTimeSetWidgetSlot(); break;
        case 4: _t->timeSetRecvMsg((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< QString(*)>(_a[5])),(*reinterpret_cast< QString(*)>(_a[6]))); break;
        case 5: _t->carNoChangeSlot(); break;
        case 6: _t->SearchBtnClicked(); break;
        case 7: _t->recordQueryEndSlot(); break;
        case 8: _t->DownBtnClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (recordManage::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&recordManage::hideRecSysPage)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject recordManage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_recordManage.data,
    qt_meta_data_recordManage,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *recordManage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *recordManage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_recordManage.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int recordManage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void recordManage::hideRecSysPage()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
