/****************************************************************************
** Meta object code from reading C++ file 'audio_backend.h'
**
** Created: Sun May 9 17:02:27 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "audio_backend.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'audio_backend.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Haruhi__AudioBackend[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x0a,
      32,   21,   21,   21, 0x0a,
      45,   21,   21,   21, 0x0a,
      62,   21,   21,   21, 0x0a,
      82,   77,   21,   21, 0x0a,
     104,   21,   21,   21, 0x0a,
     120,   77,   21,   21, 0x0a,
     143,   21,   21,   21, 0x0a,
     175,   21,   21,   21, 0x0a,
     210,  208,   21,   21, 0x0a,
     255,  208,   21,   21, 0x0a,
     301,   21,   21,   21, 0x0a,
     328,   21,   21,   21, 0x0a,
     356,   21,   21,   21, 0x0a,
     380,   21,   21,   21, 0x0a,
     405,   21,   21,   21, 0x0a,
     430,   21,   21,   21, 0x0a,
     456,   21,   21,   21, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__AudioBackend[] = {
    "Haruhi::AudioBackend\0\0connect()\0"
    "disconnect()\0update_widgets()\0"
    "create_input()\0name\0create_input(QString)\0"
    "create_output()\0create_output(QString)\0"
    "context_menu_for_inputs(QPoint)\0"
    "context_menu_for_outputs(QPoint)\0,\0"
    "double_click_on_inputs(QTreeWidgetItem*,int)\0"
    "double_click_on_outputs(QTreeWidgetItem*,int)\0"
    "configure_selected_input()\0"
    "configure_selected_output()\0"
    "rename_selected_input()\0"
    "rename_selected_output()\0"
    "destroy_selected_input()\0"
    "destroy_selected_output()\0dummy_round()\0"
};

const QMetaObject Haruhi::AudioBackend::staticMetaObject = {
    { &Unit::staticMetaObject, qt_meta_stringdata_Haruhi__AudioBackend,
      qt_meta_data_Haruhi__AudioBackend, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::AudioBackend::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::AudioBackend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::AudioBackend::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__AudioBackend))
        return static_cast<void*>(const_cast< AudioBackend*>(this));
    if (!strcmp(_clname, "SaveableState"))
        return static_cast< SaveableState*>(const_cast< AudioBackend*>(this));
    if (!strcmp(_clname, "Backend"))
        return static_cast< Backend*>(const_cast< AudioBackend*>(this));
    return Unit::qt_metacast(_clname);
}

int Haruhi::AudioBackend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Unit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connect(); break;
        case 1: disconnect(); break;
        case 2: update_widgets(); break;
        case 3: create_input(); break;
        case 4: create_input((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: create_output(); break;
        case 6: create_output((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: context_menu_for_inputs((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 8: context_menu_for_outputs((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 9: double_click_on_inputs((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 10: double_click_on_outputs((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 11: configure_selected_input(); break;
        case 12: configure_selected_output(); break;
        case 13: rename_selected_input(); break;
        case 14: rename_selected_output(); break;
        case 15: destroy_selected_input(); break;
        case 16: destroy_selected_output(); break;
        case 17: dummy_round(); break;
        default: ;
        }
        _id -= 18;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
