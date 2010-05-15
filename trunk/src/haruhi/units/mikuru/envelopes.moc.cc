/****************************************************************************
** Meta object code from reading C++ file 'envelopes.h'
**
** Created: Tue May 4 23:30:19 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "envelopes.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'envelopes.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MikuruPrivate__Envelopes[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      35,   32,   26,   25, 0x0a,
      49,   25,   26,   25, 0x2a,
      65,   32,   60,   25, 0x0a,
      78,   25,   60,   25, 0x2a,
      88,   25,   25,   25, 0x0a,
     107,   25,   25,   25, 0x0a,
     135,   25,   25,   25, 0x0a,
     159,   25,   25,   25, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MikuruPrivate__Envelopes[] = {
    "MikuruPrivate::Envelopes\0\0ADSR*\0id\0"
    "add_adsr(int)\0add_adsr()\0LFO*\0"
    "add_lfo(int)\0add_lfo()\0destroy_envelope()\0"
    "destroy_envelope(Envelope*)\0"
    "destroy_all_envelopes()\0show_first()\0"
};

const QMetaObject MikuruPrivate::Envelopes::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MikuruPrivate__Envelopes,
      qt_meta_data_MikuruPrivate__Envelopes, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MikuruPrivate::Envelopes::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MikuruPrivate::Envelopes::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MikuruPrivate::Envelopes::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MikuruPrivate__Envelopes))
        return static_cast<void*>(const_cast< Envelopes*>(this));
    return QWidget::qt_metacast(_clname);
}

int MikuruPrivate::Envelopes::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: { ADSR* _r = add_adsr((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< ADSR**>(_a[0]) = _r; }  break;
        case 1: { ADSR* _r = add_adsr();
            if (_a[0]) *reinterpret_cast< ADSR**>(_a[0]) = _r; }  break;
        case 2: { LFO* _r = add_lfo((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< LFO**>(_a[0]) = _r; }  break;
        case 3: { LFO* _r = add_lfo();
            if (_a[0]) *reinterpret_cast< LFO**>(_a[0]) = _r; }  break;
        case 4: destroy_envelope(); break;
        case 5: destroy_envelope((*reinterpret_cast< Envelope*(*)>(_a[1]))); break;
        case 6: destroy_all_envelopes(); break;
        case 7: show_first(); break;
        default: ;
        }
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
