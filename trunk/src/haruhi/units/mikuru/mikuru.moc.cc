/****************************************************************************
** Meta object code from reading C++ file 'mikuru.h'
**
** Created: Tue May 4 11:59:12 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mikuru.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mikuru.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Mikuru[] = {

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
       8,    7,    7,    7, 0x0a,
      24,    7,    7,    7, 0x0a,
      41,    7,    7,    7, 0x0a,
      98,    7,   77,    7, 0x0a,
     109,    7,    7,    7, 0x0a,
     120,    7,    7,    7, 0x0a,
     151,    7,    7,    7, 0x0a,
     167,    7,    7,    7, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Mikuru[] = {
    "Mikuru\0\0update_params()\0update_widgets()\0"
    "ensure_there_is_at_least_one_part()\0"
    "MikuruPrivate::Part*\0add_part()\0"
    "del_part()\0del_part(MikuruPrivate::Part*)\0"
    "del_all_parts()\0update_ui()\0"
};

const QMetaObject Mikuru::staticMetaObject = {
    { &Haruhi::Unit::staticMetaObject, qt_meta_stringdata_Mikuru,
      qt_meta_data_Mikuru, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Mikuru::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Mikuru::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Mikuru::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Mikuru))
        return static_cast<void*>(const_cast< Mikuru*>(this));
    if (!strcmp(_clname, "Haruhi::UnitBayAware"))
        return static_cast< Haruhi::UnitBayAware*>(const_cast< Mikuru*>(this));
    if (!strcmp(_clname, "Haruhi::Presetable"))
        return static_cast< Haruhi::Presetable*>(const_cast< Mikuru*>(this));
    if (!strcmp(_clname, "SaveableState"))
        return static_cast< SaveableState*>(const_cast< Mikuru*>(this));
    typedef Haruhi::Unit QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int Mikuru::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef Haruhi::Unit QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: update_params(); break;
        case 1: update_widgets(); break;
        case 2: ensure_there_is_at_least_one_part(); break;
        case 3: { MikuruPrivate::Part* _r = add_part();
            if (_a[0]) *reinterpret_cast< MikuruPrivate::Part**>(_a[0]) = _r; }  break;
        case 4: del_part(); break;
        case 5: del_part((*reinterpret_cast< MikuruPrivate::Part*(*)>(_a[1]))); break;
        case 6: del_all_parts(); break;
        case 7: update_ui(); break;
        default: ;
        }
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
