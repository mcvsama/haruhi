/****************************************************************************
** Meta object code from reading C++ file 'knob.h'
**
** Created: Sun May 9 17:02:27 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "knob.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'knob.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_KnobProperties[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x08,
      36,   15,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_KnobProperties[] = {
    "KnobProperties\0\0limit_min_updated()\0"
    "limit_max_updated()\0"
};

const QMetaObject KnobProperties::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_KnobProperties,
      qt_meta_data_KnobProperties, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &KnobProperties::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *KnobProperties::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *KnobProperties::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_KnobProperties))
        return static_cast<void*>(const_cast< KnobProperties*>(this));
    return QDialog::qt_metacast(_clname);
}

int KnobProperties::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: limit_min_updated(); break;
        case 1: limit_max_updated(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_Knob[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
       6,    5,    5,    5, 0x05,

 // slots: signature, parameters, type, tag, flags
      19,    5,    5,    5, 0x0a,
      27,    5,    5,    5, 0x0a,
      39,    5,    5,    5, 0x0a,
      56,    5,    5,    5, 0x0a,
      72,    5,    5,    5, 0x08,
      90,    5,    5,    5, 0x08,
     118,  108,    5,    5, 0x08,
     136,  108,    5,    5, 0x08,
     157,    5,    5,    5, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Knob[] = {
    "Knob\0\0changed(int)\0reset()\0configure()\0"
    "start_learning()\0stop_learning()\0"
    "dial_changed(int)\0spin_changed(int)\0"
    "action_id\0connect_port(int)\0"
    "disconnect_port(int)\0disconnect_from_all()\0"
};

const QMetaObject Knob::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_Knob,
      qt_meta_data_Knob, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Knob::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Knob::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Knob::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Knob))
        return static_cast<void*>(const_cast< Knob*>(this));
    if (!strcmp(_clname, "ControllerProxy::Widget"))
        return static_cast< ControllerProxy::Widget*>(const_cast< Knob*>(this));
    if (!strcmp(_clname, "Haruhi::EventBackend::Learnable"))
        return static_cast< Haruhi::EventBackend::Learnable*>(const_cast< Knob*>(this));
    return QFrame::qt_metacast(_clname);
}

int Knob::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: reset(); break;
        case 2: configure(); break;
        case 3: start_learning(); break;
        case 4: stop_learning(); break;
        case 5: dial_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: spin_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: connect_port((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: disconnect_port((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: disconnect_from_all(); break;
        default: ;
        }
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Knob::changed(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
