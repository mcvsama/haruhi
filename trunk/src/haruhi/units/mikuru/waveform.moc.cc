/****************************************************************************
** Meta object code from reading C++ file 'waveform.h'
**
** Created: Tue May 4 21:48:48 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "waveform.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'waveform.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MikuruPrivate__Slider[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      23,   22,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MikuruPrivate__Slider[] = {
    "MikuruPrivate::Slider\0\0reset()\0"
};

const QMetaObject MikuruPrivate::Slider::staticMetaObject = {
    { &QSlider::staticMetaObject, qt_meta_stringdata_MikuruPrivate__Slider,
      qt_meta_data_MikuruPrivate__Slider, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MikuruPrivate::Slider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MikuruPrivate::Slider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MikuruPrivate::Slider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MikuruPrivate__Slider))
        return static_cast<void*>(const_cast< Slider*>(this));
    return QSlider::qt_metacast(_clname);
}

int MikuruPrivate::Slider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSlider::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: reset(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_MikuruPrivate__Waveform[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      25,   24,   24,   24, 0x0a,
      43,   24,   24,   24, 0x0a,
      64,   57,   24,   24, 0x0a,
      95,   24,   24,   24, 0x0a,
     111,   24,   24,   24, 0x0a,
     128,   24,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MikuruPrivate__Waveform[] = {
    "MikuruPrivate::Waveform\0\0sliders_updated()\0"
    "load_params()\0params\0"
    "load_params(Params::Waveform&)\0"
    "update_params()\0update_widgets()\0"
    "recompute_wave()\0"
};

const QMetaObject MikuruPrivate::Waveform::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MikuruPrivate__Waveform,
      qt_meta_data_MikuruPrivate__Waveform, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MikuruPrivate::Waveform::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MikuruPrivate::Waveform::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MikuruPrivate::Waveform::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MikuruPrivate__Waveform))
        return static_cast<void*>(const_cast< Waveform*>(this));
    if (!strcmp(_clname, "Signal::Receiver"))
        return static_cast< Signal::Receiver*>(const_cast< Waveform*>(this));
    return QWidget::qt_metacast(_clname);
}

int MikuruPrivate::Waveform::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: sliders_updated(); break;
        case 1: load_params(); break;
        case 2: load_params((*reinterpret_cast< Params::Waveform(*)>(_a[1]))); break;
        case 3: update_params(); break;
        case 4: update_widgets(); break;
        case 5: recompute_wave(); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
