/****************************************************************************
** Meta object code from reading C++ file 'event_backend.h'
**
** Created: Sat May 8 17:01:26 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "event_backend.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'event_backend.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Haruhi__EventBackend[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x0a,
      39,   21,   21,   21, 0x0a,
      59,   21,   21,   21, 0x0a,
      83,   21,   21,   21, 0x0a,
     107,   21,   21,   21, 0x0a,
     139,   21,   21,   21, 0x0a,
     166,   21,   21,   21, 0x0a,
     184,   21,   21,   21, 0x0a,
     209,   21,   21,   21, 0x0a,
     244,  231,   21,   21, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__EventBackend[] = {
    "Haruhi::EventBackend\0\0update_widgets()\0"
    "selection_changed()\0create_external_input()\0"
    "create_internal_input()\0"
    "context_menu_for_inputs(QPoint)\0"
    "configure_selected_input()\0learn_from_midi()\0"
    "destroy_selected_input()\0save_selected_input()\0"
    "menu_item_id\0insert_template(int)\0"
};

const QMetaObject Haruhi::EventBackend::staticMetaObject = {
    { &Unit::staticMetaObject, qt_meta_stringdata_Haruhi__EventBackend,
      qt_meta_data_Haruhi__EventBackend, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::EventBackend::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::EventBackend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::EventBackend::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__EventBackend))
        return static_cast<void*>(const_cast< EventBackend*>(this));
    if (!strcmp(_clname, "SaveableState"))
        return static_cast< SaveableState*>(const_cast< EventBackend*>(this));
    if (!strcmp(_clname, "Backend"))
        return static_cast< Backend*>(const_cast< EventBackend*>(this));
    return Unit::qt_metacast(_clname);
}

int Haruhi::EventBackend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Unit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: update_widgets(); break;
        case 1: selection_changed(); break;
        case 2: create_external_input(); break;
        case 3: create_internal_input(); break;
        case 4: context_menu_for_inputs((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 5: configure_selected_input(); break;
        case 6: learn_from_midi(); break;
        case 7: destroy_selected_input(); break;
        case 8: save_selected_input(); break;
        case 9: insert_template((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
