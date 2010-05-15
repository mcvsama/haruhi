/****************************************************************************
** Meta object code from reading C++ file 'preset_editor.h'
**
** Created: Tue May 4 11:59:12 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "preset_editor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'preset_editor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Haruhi__PresetsManagerPrivate__PresetEditor[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      45,   44,   44,   44, 0x05,

 // slots: signature, parameters, type, tag, flags
      63,   44,   44,   44, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__PresetsManagerPrivate__PresetEditor[] = {
    "Haruhi::PresetsManagerPrivate::PresetEditor\0"
    "\0details_updated()\0update_details()\0"
};

const QMetaObject Haruhi::PresetsManagerPrivate::PresetEditor::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Haruhi__PresetsManagerPrivate__PresetEditor,
      qt_meta_data_Haruhi__PresetsManagerPrivate__PresetEditor, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::PresetsManagerPrivate::PresetEditor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::PresetsManagerPrivate::PresetEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::PresetsManagerPrivate::PresetEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__PresetsManagerPrivate__PresetEditor))
        return static_cast<void*>(const_cast< PresetEditor*>(this));
    return QWidget::qt_metacast(_clname);
}

int Haruhi::PresetsManagerPrivate::PresetEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: details_updated(); break;
        case 1: update_details(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Haruhi::PresetsManagerPrivate::PresetEditor::details_updated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
