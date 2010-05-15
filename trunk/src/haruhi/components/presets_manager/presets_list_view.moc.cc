/****************************************************************************
** Meta object code from reading C++ file 'presets_list_view.h'
**
** Created: Wed May 5 21:28:31 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "presets_list_view.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'presets_list_view.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Haruhi__PresetsManagerPrivate__PresetsListView[] = {

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
      48,   47,   47,   47, 0x08,
      82,   69,   47,   47, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__PresetsManagerPrivate__PresetsListView[] = {
    "Haruhi::PresetsManagerPrivate::PresetsListView\0"
    "\0auto_open_selected()\0item,pos,col\0"
    "context_menu(Q3ListViewItem*,QPoint,int)\0"
};

const QMetaObject Haruhi::PresetsManagerPrivate::PresetsListView::staticMetaObject = {
    { &Q3ListView::staticMetaObject, qt_meta_stringdata_Haruhi__PresetsManagerPrivate__PresetsListView,
      qt_meta_data_Haruhi__PresetsManagerPrivate__PresetsListView, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::PresetsManagerPrivate::PresetsListView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::PresetsManagerPrivate::PresetsListView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::PresetsManagerPrivate::PresetsListView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__PresetsManagerPrivate__PresetsListView))
        return static_cast<void*>(const_cast< PresetsListView*>(this));
    return Q3ListView::qt_metacast(_clname);
}

int Haruhi::PresetsManagerPrivate::PresetsListView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3ListView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: auto_open_selected(); break;
        case 1: context_menu((*reinterpret_cast< Q3ListViewItem*(*)>(_a[1])),(*reinterpret_cast< const QPoint(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
