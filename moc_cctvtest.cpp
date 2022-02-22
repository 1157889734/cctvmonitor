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
    QByteArrayData data[21];
    char stringdata0[304];
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
QT_MOC_LITERAL(5, 54, 12), // "showcctvPage"
QT_MOC_LITERAL(6, 67, 15), // "showMonitorPage"
QT_MOC_LITERAL(7, 83, 14), // "sigalePageSlot"
QT_MOC_LITERAL(8, 98, 12), // "fourPageSlot"
QT_MOC_LITERAL(9, 111, 9), // "cycleSlot"
QT_MOC_LITERAL(10, 121, 14), // "timeupdateSlot"
QT_MOC_LITERAL(11, 136, 18), // "updateWarnInfoSLot"
QT_MOC_LITERAL(12, 155, 13), // "PlayWidCicked"
QT_MOC_LITERAL(13, 169, 20), // "GroupButtonClickSlot"
QT_MOC_LITERAL(14, 190, 16), // "QAbstractButton*"
QT_MOC_LITERAL(15, 207, 3), // "btn"
QT_MOC_LITERAL(16, 211, 19), // "GroupButtonFireSlot"
QT_MOC_LITERAL(17, 231, 19), // "GroupButtonDoorSlot"
QT_MOC_LITERAL(18, 251, 23), // "GroupButtonDoorclipSlot"
QT_MOC_LITERAL(19, 275, 19), // "GroupButtonPecuSlot"
QT_MOC_LITERAL(20, 295, 8) // "playSlot"

    },
    "cctvTest\0showMonitorSignal\0\0"
    "sendWindIndexSignal\0index\0showcctvPage\0"
    "showMonitorPage\0sigalePageSlot\0"
    "fourPageSlot\0cycleSlot\0timeupdateSlot\0"
    "updateWarnInfoSLot\0PlayWidCicked\0"
    "GroupButtonClickSlot\0QAbstractButton*\0"
    "btn\0GroupButtonFireSlot\0GroupButtonDoorSlot\0"
    "GroupButtonDoorclipSlot\0GroupButtonPecuSlot\0"
    "playSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_cctvTest[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x06 /* Public */,
       3,    1,   95,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   98,    2, 0x0a /* Public */,
       6,    0,   99,    2, 0x0a /* Public */,
       7,    0,  100,    2, 0x0a /* Public */,
       8,    0,  101,    2, 0x0a /* Public */,
       9,    0,  102,    2, 0x0a /* Public */,
      10,    0,  103,    2, 0x0a /* Public */,
      11,    0,  104,    2, 0x0a /* Public */,
      12,    1,  105,    2, 0x0a /* Public */,
      13,    1,  108,    2, 0x0a /* Public */,
      16,    1,  111,    2, 0x0a /* Public */,
      17,    1,  114,    2, 0x0a /* Public */,
      18,    1,  117,    2, 0x0a /* Public */,
      19,    1,  120,    2, 0x0a /* Public */,
      20,    0,  123,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
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
        case 2: _t->showcctvPage(); break;
        case 3: _t->showMonitorPage(); break;
        case 4: _t->sigalePageSlot(); break;
        case 5: _t->fourPageSlot(); break;
        case 6: _t->cycleSlot(); break;
        case 7: _t->timeupdateSlot(); break;
        case 8: _t->updateWarnInfoSLot(); break;
        case 9: _t->PlayWidCicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->GroupButtonClickSlot((*reinterpret_cast< QAbstractButton*(*)>(_a[1]))); break;
        case 11: _t->GroupButtonFireSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->GroupButtonDoorSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->GroupButtonDoorclipSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->GroupButtonPecuSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->playSlot(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 10:
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
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
