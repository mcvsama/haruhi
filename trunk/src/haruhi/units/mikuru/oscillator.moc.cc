/****************************************************************************
** Meta object code from reading C++ file 'oscillator.h'
**
** Created: Tue May 4 11:59:13 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "oscillator.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'oscillator.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MikuruPrivate__Oscillator[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x0a,
      52,   26,   26,   26, 0x0a,
      72,   26,   26,   26, 0x0a,
      95,   88,   26,   26, 0x0a,
     139,   88,   26,   26, 0x0a,
     173,   26,   26,   26, 0x0a,
     200,   26,   26,   26, 0x0a,
     222,   26,   26,   26, 0x0a,
     239,   26,   26,   26, 0x08,
     263,   26,   26,   26, 0x08,
     285,   26,   26,   26, 0x08,
     310,   26,   26,   26, 0x08,
     338,   26,   26,   26, 0x08,
     367,   26,   26,   26, 0x08,
     394,   26,   26,   26, 0x08,
     422,   26,   26,   26, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MikuruPrivate__Oscillator[] = {
    "MikuruPrivate::Oscillator\0\0"
    "load_oscillator_params()\0load_voice_params()\0"
    "update_params()\0params\0"
    "load_oscillator_params(Params::Oscillator&)\0"
    "load_voice_params(Params::Voice&)\0"
    "update_oscillator_params()\0"
    "update_voice_params()\0update_widgets()\0"
    "update_voice_panorama()\0update_voice_detune()\0"
    "update_voice_pitchbend()\0"
    "update_voice_unison_index()\0"
    "update_voice_unison_spread()\0"
    "update_voice_unison_init()\0"
    "update_voice_unison_noise()\0"
    "update_voice_velocity_sens()\0"
};

const QMetaObject MikuruPrivate::Oscillator::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MikuruPrivate__Oscillator,
      qt_meta_data_MikuruPrivate__Oscillator, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MikuruPrivate::Oscillator::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MikuruPrivate::Oscillator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MikuruPrivate::Oscillator::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MikuruPrivate__Oscillator))
        return static_cast<void*>(const_cast< Oscillator*>(this));
    return QWidget::qt_metacast(_clname);
}

int MikuruPrivate::Oscillator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: load_oscillator_params(); break;
        case 1: load_voice_params(); break;
        case 2: update_params(); break;
        case 3: load_oscillator_params((*reinterpret_cast< Params::Oscillator(*)>(_a[1]))); break;
        case 4: load_voice_params((*reinterpret_cast< Params::Voice(*)>(_a[1]))); break;
        case 5: update_oscillator_params(); break;
        case 6: update_voice_params(); break;
        case 7: update_widgets(); break;
        case 8: update_voice_panorama(); break;
        case 9: update_voice_detune(); break;
        case 10: update_voice_pitchbend(); break;
        case 11: update_voice_unison_index(); break;
        case 12: update_voice_unison_spread(); break;
        case 13: update_voice_unison_init(); break;
        case 14: update_voice_unison_noise(); break;
        case 15: update_voice_velocity_sens(); break;
        default: ;
        }
        _id -= 16;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
