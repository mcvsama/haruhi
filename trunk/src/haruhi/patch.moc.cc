/****************************************************************************
** Meta object code from reading C++ file 'patch.h'
**
** Created: Tue May 4 11:59:13 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "patch.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'patch.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Haruhi__PatchPrivate__ConnectionsTab[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__PatchPrivate__ConnectionsTab[] = {
    "Haruhi::PatchPrivate::ConnectionsTab\0"
};

const QMetaObject Haruhi::PatchPrivate::ConnectionsTab::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Haruhi__PatchPrivate__ConnectionsTab,
      qt_meta_data_Haruhi__PatchPrivate__ConnectionsTab, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::PatchPrivate::ConnectionsTab::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::PatchPrivate::ConnectionsTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::PatchPrivate::ConnectionsTab::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__PatchPrivate__ConnectionsTab))
        return static_cast<void*>(const_cast< ConnectionsTab*>(this));
    return QWidget::qt_metacast(_clname);
}

int Haruhi::PatchPrivate::ConnectionsTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_Haruhi__PatchPrivate__UnitTab[] = {

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
      31,   30,   30,   30, 0x0a,
      40,   30,   30,   30, 0x0a,
      60,   50,   30,   30, 0x0a,
      88,   30,   30,   30, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__PatchPrivate__UnitTab[] = {
    "Haruhi::PatchPrivate::UnitTab\0\0unload()\0"
    "presets()\0uuid,name\0set_preset(QString,QString)\0"
    "favorited(bool)\0"
};

const QMetaObject Haruhi::PatchPrivate::UnitTab::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Haruhi__PatchPrivate__UnitTab,
      qt_meta_data_Haruhi__PatchPrivate__UnitTab, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::PatchPrivate::UnitTab::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::PatchPrivate::UnitTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::PatchPrivate::UnitTab::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__PatchPrivate__UnitTab))
        return static_cast<void*>(const_cast< UnitTab*>(this));
    return QWidget::qt_metacast(_clname);
}

int Haruhi::PatchPrivate::UnitTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: unload(); break;
        case 1: presets(); break;
        case 2: set_preset((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 3: favorited((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
static const uint qt_meta_data_Haruhi__Patch[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   15,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__Patch[] = {
    "Haruhi::Patch\0\0i\0load_unit_request(int)\0"
};

const QMetaObject Haruhi::Patch::staticMetaObject = {
    { &UnitBay::staticMetaObject, qt_meta_stringdata_Haruhi__Patch,
      qt_meta_data_Haruhi__Patch, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::Patch::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::Patch::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::Patch::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__Patch))
        return static_cast<void*>(const_cast< Patch*>(this));
    if (!strcmp(_clname, "SaveableState"))
        return static_cast< SaveableState*>(const_cast< Patch*>(this));
    return UnitBay::qt_metacast(_clname);
}

int Haruhi::Patch::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = UnitBay::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: load_unit_request((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
