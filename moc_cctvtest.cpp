/****************************************************************************
** Meta object code from reading C++ file 'cctvtest.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "cctvtest.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cctvtest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_cctvTest_t {
    QByteArrayData data[25];
    char stringdata0[390];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_cctvTest_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_cctvTest_t qt_meta_stringdata_cctvTest = {
    {
QT_MOC_LITERAL(0, 0, 8), // "cctvTest"
QT_MOC_LITERAL(1, 9, 17), // "showMonitorSignal"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 19), // "sendWindIndexSignal"
QT_MOC_LITERAL(4, 48, 5), // "index"
QT_MOC_LITERAL(5, 54, 17), // "getDevStateSignal"
QT_MOC_LITERAL(6, 72, 13), // "setTimeSignal"
QT_MOC_LITERAL(7, 86, 14), // "sendPLaySignal"
QT_MOC_LITERAL(8, 101, 12), // "showcctvPage"
QT_MOC_LITERAL(9, 114, 21), // "showMonitorManagePage"
QT_MOC_LITERAL(10, 136, 14), // "sigalePageSlot"
QT_MOC_LITERAL(11, 151, 12), // "fourPageSlot"
QT_MOC_LITERAL(12, 164, 9), // "cycleSlot"
QT_MOC_LITERAL(13, 174, 14), // "timeupdateSlot"
QT_MOC_LITERAL(14, 189, 13), // "PlayWidCicked"
QT_MOC_LITERAL(15, 203, 25), // "GroupButtonVideoClickSlot"
QT_MOC_LITERAL(16, 229, 16), // "QAbstractButton*"
QT_MOC_LITERAL(17, 246, 3), // "btn"
QT_MOC_LITERAL(18, 250, 19), // "GroupButtonFireSlot"
QT_MOC_LITERAL(19, 270, 19), // "GroupButtonDoorSlot"
QT_MOC_LITERAL(20, 290, 23), // "GroupButtonDoorclipSlot"
QT_MOC_LITERAL(21, 314, 19), // "GroupButtonPecuSlot"
QT_MOC_LITERAL(22, 334, 15), // "PlayCtrlFunSlot"
QT_MOC_LITERAL(23, 350, 18), // "updateWarnInfoSLot"
QT_MOC_LITERAL(24, 369, 20) // "videoPollingfunction"

    },
    "cctvTest\0showMonitorSignal\0\0"
    "sendWindIndexSignal\0index\0getDevStateSignal\0"
    "setTimeSignal\0sendPLaySignal\0showcctvPage\0"
    "showMonitorManagePage\0sigalePageSlot\0"
    "fourPageSlot\0cycleSlot\0timeupdateSlot\0"
    "PlayWidCicked\0GroupButtonVideoClickSlot\0"
    "QAbstractButton*\0btn\0GroupButtonFireSlot\0"
    "GroupButtonDoorSlot\0GroupButtonDoorclipSlot\0"
    "GroupButtonPecuSlot\0PlayCtrlFunSlot\0"
    "updateWarnInfoSLot\0videoPollingfunction"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_cctvTest[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  114,    2, 0x06 /* Public */,
       3,    1,  115,    2, 0x06 /* Public */,
       5,    0,  118,    2, 0x06 /* Public */,
       6,    0,  119,    2, 0x06 /* Public */,
       7,    0,  120,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,  121,    2, 0x0a /* Public */,
       9,    0,  122,    2, 0x0a /* Public */,
      10,    0,  123,    2, 0x0a /* Public */,
      11,    0,  124,    2, 0x0a /* Public */,
      12,    0,  125,    2, 0x0a /* Public */,
      13,    0,  126,    2, 0x0a /* Public */,
      14,    1,  127,    2, 0x0a /* Public */,
      15,    1,  130,    2, 0x0a /* Public */,
      18,    1,  133,    2, 0x0a /* Public */,
      19,    1,  136,    2, 0x0a /* Public */,
      20,    1,  139,    2, 0x0a /* Public */,
      21,    1,  142,    2, 0x0a /* Public */,
      22,    0,  145,    2, 0x0a /* Public */,
      23,    0,  146,    2, 0x0a /* Public */,
      24,    0,  147,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void cctvTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<cctvTest *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->showMonitorSignal(); break;
        case 1: _t->sendWindIndexSignal((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->getDevStateSignal(); break;
        case 3: _t->setTimeSignal(); break;
        case 4: _t->sendPLaySignal(); break;
        case 5: _t->showcctvPage(); break;
        case 6: _t->showMonitorManagePage(); break;
        case 7: _t->sigalePageSlot(); break;
        case 8: _t->fourPageSlot(); break;
        case 9: _t->cycleSlot(); break;
        case 10: _t->timeupdateSlot(); break;
        case 11: _t->PlayWidCicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->GroupButtonVideoClickSlot((*reinterpret_cast< QAbstractButton*(*)>(_a[1]))); break;
        case 13: _t->GroupButtonFireSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->GroupButtonDoorSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->GroupButtonDoorclipSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->GroupButtonPecuSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->PlayCtrlFunSlot(); break;
        case 18: _t->updateWarnInfoSLot(); break;
        case 19: _t->videoPollingfunction(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 12:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractButton* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (cctvTest::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&cctvTest::showMonitorSignal)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (cctvTest::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&cctvTest::sendWindIndexSignal)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (cctvTest::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&cctvTest::getDevStateSignal)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (cctvTest::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&cctvTest::setTimeSignal)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (cctvTest::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&cctvTest::sendPLaySignal)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject cctvTest::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_cctvTest.data,
    qt_meta_data_cctvTest,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *cctvTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *cctvTest::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_cctvTest.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int cctvTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}

// SIGNAL 0
void cctvTest::showMonitorSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void cctvTest::sendWindIndexSignal(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void cctvTest::getDevStateSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void cctvTest::setTimeSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void cctvTest::sendPLaySignal()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
