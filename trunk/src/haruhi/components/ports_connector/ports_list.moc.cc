/****************************************************************************
** Meta object code from reading C++ file 'ports_list.h'
**
** Created: Wed May 5 21:28:31 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ports_list.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ports_list.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Haruhi__PortsConnectorPrivate__PortsList[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      44,   42,   41,   41, 0x05,

 // slots: signature, parameters, type, tag, flags
      82,   41,   41,   41, 0x0a,
     101,   41,   41,   41, 0x0a,
     120,   41,   41,   41, 0x0a,
     144,  139,   41,   41, 0x0a,
     167,   41,  162,   41, 0x0a,
     185,   41,   41,   41, 0x0a,
     206,   41,   41,   41, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__PortsConnectorPrivate__PortsList[] = {
    "Haruhi::PortsConnectorPrivate::PortsList\0"
    "\0,\0context_menu(QTreeWidgetItem*,QPoint)\0"
    "insert_unit(Unit*)\0remove_unit(Unit*)\0"
    "update_unit(Unit*)\0unit\0set_filter(Unit*)\0"
    "bool\0unit_exist(Unit*)\0auto_open_selected()\0"
    "context_menu(QPoint)\0"
};

const QMetaObject Haruhi::PortsConnectorPrivate::PortsList::staticMetaObject = {
    { &QTreeWidget::staticMetaObject, qt_meta_stringdata_Haruhi__PortsConnectorPrivate__PortsList,
      qt_meta_data_Haruhi__PortsConnectorPrivate__PortsList, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::PortsConnectorPrivate::PortsList::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::PortsConnectorPrivate::PortsList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::PortsConnectorPrivate::PortsList::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__PortsConnectorPrivate__PortsList))
        return static_cast<void*>(const_cast< PortsList*>(this));
    return QTreeWidget::qt_metacast(_clname);
}

int Haruhi::PortsConnectorPrivate::PortsList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: context_menu((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< const QPoint(*)>(_a[2]))); break;
        case 1: insert_unit((*reinterpret_cast< Unit*(*)>(_a[1]))); break;
        case 2: remove_unit((*reinterpret_cast< Unit*(*)>(_a[1]))); break;
        case 3: update_unit((*reinterpret_cast< Unit*(*)>(_a[1]))); break;
        case 4: set_filter((*reinterpret_cast< Unit*(*)>(_a[1]))); break;
        case 5: { bool _r = unit_exist((*reinterpret_cast< Unit*(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 6: auto_open_selected(); break;
        case 7: context_menu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void Haruhi::PortsConnectorPrivate::PortsList::context_menu(QTreeWidgetItem * _t1, QPoint const & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
