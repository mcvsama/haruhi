/****************************************************************************
** Meta object code from reading C++ file 'presets_manager.h'
**
** Created: Tue May 4 11:59:12 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "presets_manager.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'presets_manager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Haruhi__PresetsManager[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      48,   24,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
      81,   23,   23,   23, 0x0a,
      95,   23,   23,   23, 0x0a,
     124,   23,   23,   23, 0x0a,
     138,   23,   23,   23, 0x0a,
     155,   23,   23,   23, 0x0a,
     173,   23,   23,   23, 0x0a,
     189,   23,   23,   23, 0x0a,
     230,  211,  199,   23, 0x0a,
     275,   23,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__PresetsManager[] = {
    "Haruhi::PresetsManager\0\0preset_uuid,preset_name\0"
    "preset_selected(QString,QString)\0"
    "load_preset()\0load_preset(Q3ListViewItem*)\0"
    "save_preset()\0create_package()\0"
    "create_category()\0create_preset()\0"
    "destroy()\0QDomElement\0subject,name,value\0"
    "append_element(QDomElement&,QString,QString)\0"
    "update_widgets()\0"
};

const QMetaObject Haruhi::PresetsManager::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Haruhi__PresetsManager,
      qt_meta_data_Haruhi__PresetsManager, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::PresetsManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::PresetsManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::PresetsManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__PresetsManager))
        return static_cast<void*>(const_cast< PresetsManager*>(this));
    return QWidget::qt_metacast(_clname);
}

int Haruhi::PresetsManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: preset_selected((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: load_preset(); break;
        case 2: load_preset((*reinterpret_cast< Q3ListViewItem*(*)>(_a[1]))); break;
        case 3: save_preset(); break;
        case 4: create_package(); break;
        case 5: create_category(); break;
        case 6: create_preset(); break;
        case 7: destroy(); break;
        case 8: { QDomElement _r = append_element((*reinterpret_cast< QDomElement(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< QDomElement*>(_a[0]) = _r; }  break;
        case 9: update_widgets(); break;
        default: ;
        }
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Haruhi::PresetsManager::preset_selected(QString const & _t1, QString const & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
