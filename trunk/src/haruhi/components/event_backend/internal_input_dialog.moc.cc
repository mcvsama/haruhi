/****************************************************************************
** Meta object code from reading C++ file 'internal_input_dialog.h'
**
** Created: Tue May 4 11:59:12 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "internal_input_dialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'internal_input_dialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Haruhi__EventBackendPrivate__InternalInputDialog[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      50,   49,   49,   49, 0x08,
      67,   49,   49,   49, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__EventBackendPrivate__InternalInputDialog[] = {
    "Haruhi::EventBackendPrivate::InternalInputDialog\0"
    "\0update_widgets()\0validate_and_save()\0"
};

const QMetaObject Haruhi::EventBackendPrivate::InternalInputDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Haruhi__EventBackendPrivate__InternalInputDialog,
      qt_meta_data_Haruhi__EventBackendPrivate__InternalInputDialog, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::EventBackendPrivate::InternalInputDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::EventBackendPrivate::InternalInputDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::EventBackendPrivate::InternalInputDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__EventBackendPrivate__InternalInputDialog))
        return static_cast<void*>(const_cast< InternalInputDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int Haruhi::EventBackendPrivate::InternalInputDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: update_widgets(); break;
        case 1: validate_and_save(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
