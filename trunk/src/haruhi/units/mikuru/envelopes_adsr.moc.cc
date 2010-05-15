/****************************************************************************
** Meta object code from reading C++ file 'envelopes_adsr.h'
**
** Created: Tue May 4 19:13:59 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "envelopes_adsr.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'envelopes_adsr.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MikuruPrivate__ADSR[] = {

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
      21,   20,   20,   20, 0x0a,
      42,   35,   20,   20, 0x0a,
      69,   20,   20,   20, 0x0a,
      85,   20,   20,   20, 0x0a,
     104,   99,   20,   20, 0x0a,
     124,   20,   20,   20, 0x2a,

       0        // eod
};

static const char qt_meta_stringdata_MikuruPrivate__ADSR[] = {
    "MikuruPrivate::ADSR\0\0load_params()\0"
    "params\0load_params(Params::ADSR&)\0"
    "update_params()\0update_plot()\0part\0"
    "auto_connect(Part*)\0auto_connect()\0"
};

const QMetaObject MikuruPrivate::ADSR::staticMetaObject = {
    { &Envelope::staticMetaObject, qt_meta_stringdata_MikuruPrivate__ADSR,
      qt_meta_data_MikuruPrivate__ADSR, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MikuruPrivate::ADSR::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MikuruPrivate::ADSR::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MikuruPrivate::ADSR::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MikuruPrivate__ADSR))
        return static_cast<void*>(const_cast< ADSR*>(this));
    return Envelope::qt_metacast(_clname);
}

int MikuruPrivate::ADSR::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Envelope::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: load_params(); break;
        case 1: load_params((*reinterpret_cast< Params::ADSR(*)>(_a[1]))); break;
        case 2: update_params(); break;
        case 3: update_plot(); break;
        case 4: auto_connect((*reinterpret_cast< Part*(*)>(_a[1]))); break;
        case 5: auto_connect(); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
