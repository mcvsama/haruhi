/****************************************************************************
** Meta object code from reading C++ file 'envelopes_lfo.h'
**
** Created: Tue May 4 21:48:48 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "envelopes_lfo.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'envelopes_lfo.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MikuruPrivate__LFO[] = {

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
      20,   19,   19,   19, 0x0a,
      41,   34,   19,   19, 0x0a,
      67,   19,   19,   19, 0x0a,
      83,   19,   19,   19, 0x0a,
      97,   19,   19,   19, 0x0a,
     114,   19,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MikuruPrivate__LFO[] = {
    "MikuruPrivate::LFO\0\0load_params()\0"
    "params\0load_params(Params::LFO&)\0"
    "update_params()\0update_plot()\0"
    "update_widgets()\0update_wave_param()\0"
};

const QMetaObject MikuruPrivate::LFO::staticMetaObject = {
    { &Envelope::staticMetaObject, qt_meta_stringdata_MikuruPrivate__LFO,
      qt_meta_data_MikuruPrivate__LFO, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MikuruPrivate::LFO::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MikuruPrivate::LFO::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MikuruPrivate::LFO::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MikuruPrivate__LFO))
        return static_cast<void*>(const_cast< LFO*>(this));
    return Envelope::qt_metacast(_clname);
}

int MikuruPrivate::LFO::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Envelope::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: load_params(); break;
        case 1: load_params((*reinterpret_cast< Params::LFO(*)>(_a[1]))); break;
        case 2: update_params(); break;
        case 3: update_plot(); break;
        case 4: update_widgets(); break;
        case 5: update_wave_param(); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
