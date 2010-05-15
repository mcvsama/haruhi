/****************************************************************************
** Meta object code from reading C++ file 'session_loader.h'
**
** Created: Tue May 4 11:59:13 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "session_loader.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'session_loader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Haruhi__SessionLoader[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      23,   22,   22,   22, 0x08,
      40,   22,   22,   22, 0x08,
      62,   22,   22,   22, 0x08,
      78,   76,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__SessionLoader[] = {
    "Haruhi::SessionLoader\0\0update_widgets()\0"
    "validate_and_accept()\0browse_file()\0"
    ",\0open_recent(QTreeWidgetItem*,int)\0"
};

const QMetaObject Haruhi::SessionLoader::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Haruhi__SessionLoader,
      qt_meta_data_Haruhi__SessionLoader, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::SessionLoader::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::SessionLoader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::SessionLoader::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__SessionLoader))
        return static_cast<void*>(const_cast< SessionLoader*>(this));
    return QDialog::qt_metacast(_clname);
}

int Haruhi::SessionLoader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: update_widgets(); break;
        case 1: validate_and_accept(); break;
        case 2: browse_file(); break;
        case 3: open_recent((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
