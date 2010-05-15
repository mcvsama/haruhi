/****************************************************************************
** Meta object code from reading C++ file 'waveshaper.h'
**
** Created: Tue May 4 11:59:13 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "waveshaper.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'waveshaper.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MikuruPrivate__Waveshaper[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x0a,
      41,   26,   26,   26, 0x0a,
      57,   26,   26,   26, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MikuruPrivate__Waveshaper[] = {
    "MikuruPrivate::Waveshaper\0\0load_params()\0"
    "update_params()\0update_widgets()\0"
};

const QMetaObject MikuruPrivate::Waveshaper::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MikuruPrivate__Waveshaper,
      qt_meta_data_MikuruPrivate__Waveshaper, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MikuruPrivate::Waveshaper::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MikuruPrivate::Waveshaper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MikuruPrivate::Waveshaper::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MikuruPrivate__Waveshaper))
        return static_cast<void*>(const_cast< Waveshaper*>(this));
    return QWidget::qt_metacast(_clname);
}

int MikuruPrivate::Waveshaper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: load_params(); break;
        case 1: update_params(); break;
        case 2: update_widgets(); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
