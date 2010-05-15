/****************************************************************************
** Meta object code from reading C++ file 'general.h'
**
** Created: Tue May 4 11:59:12 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "general.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'general.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MikuruPrivate__General[] = {

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
      24,   23,   23,   23, 0x0a,
      45,   38,   23,   23, 0x0a,
      75,   23,   23,   23, 0x0a,
      99,   91,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MikuruPrivate__General[] = {
    "MikuruPrivate::General\0\0load_params()\0"
    "params\0load_params(Params::General&)\0"
    "update_params()\0threads\0update_threads(int)\0"
};

const QMetaObject MikuruPrivate::General::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MikuruPrivate__General,
      qt_meta_data_MikuruPrivate__General, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MikuruPrivate::General::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MikuruPrivate::General::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MikuruPrivate::General::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MikuruPrivate__General))
        return static_cast<void*>(const_cast< General*>(this));
    return QWidget::qt_metacast(_clname);
}

int MikuruPrivate::General::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: load_params(); break;
        case 1: load_params((*reinterpret_cast< Params::General(*)>(_a[1]))); break;
        case 2: update_params(); break;
        case 3: update_threads((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
