/****************************************************************************
** Meta object code from reading C++ file 'ports_connector.h'
**
** Created: Sat May 8 22:49:37 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ports_connector.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ports_connector.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Haruhi__PortsConnector[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x0a,
      38,   23,   23,   23, 0x0a,
      62,   53,   23,   23, 0x0a,
     100,   23,   23,   23, 0x0a,
     118,   23,   23,   23, 0x0a,
     138,   23,   23,   23, 0x0a,
     183,   23,   23,   23, 0x0a,
     203,   23,   23,   23, 0x0a,
     222,   23,   23,   23, 0x0a,
     244,   23,   23,   23, 0x0a,
     275,   23,   23,   23, 0x0a,
     294,   23,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__PortsConnector[] = {
    "Haruhi::PortsConnector\0\0create_port()\0"
    "destroy_port()\0item,pos\0"
    "context_menu(QTreeWidgetItem*,QPoint)\0"
    "list_view_moved()\0list_view_changed()\0"
    "item_expanded_or_collapsed(QTreeWidgetItem*)\0"
    "selection_changed()\0connect_selected()\0"
    "disconnect_selected()\0"
    "disconnect_all_from_selected()\0"
    "insert_unit(Unit*)\0remove_unit(Unit*)\0"
};

const QMetaObject Haruhi::PortsConnector::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Haruhi__PortsConnector,
      qt_meta_data_Haruhi__PortsConnector, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::PortsConnector::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::PortsConnector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::PortsConnector::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__PortsConnector))
        return static_cast<void*>(const_cast< PortsConnector*>(this));
    if (!strcmp(_clname, "Signal::Receiver"))
        return static_cast< Signal::Receiver*>(const_cast< PortsConnector*>(this));
    return QWidget::qt_metacast(_clname);
}

int Haruhi::PortsConnector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: create_port(); break;
        case 1: destroy_port(); break;
        case 2: context_menu((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< const QPoint(*)>(_a[2]))); break;
        case 3: list_view_moved(); break;
        case 4: list_view_changed(); break;
        case 5: item_expanded_or_collapsed((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1]))); break;
        case 6: selection_changed(); break;
        case 7: connect_selected(); break;
        case 8: disconnect_selected(); break;
        case 9: disconnect_all_from_selected(); break;
        case 10: insert_unit((*reinterpret_cast< Unit*(*)>(_a[1]))); break;
        case 11: remove_unit((*reinterpret_cast< Unit*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
