/****************************************************************************
** Meta object code from reading C++ file 'session.h'
**
** Created: Tue May 4 11:59:13 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "session.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'session.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Haruhi__SessionPrivate__SettingsDialog[] = {

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
      40,   39,   39,   39, 0x08,
      56,   39,   39,   39, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__SessionPrivate__SettingsDialog[] = {
    "Haruhi::SessionPrivate::SettingsDialog\0"
    "\0state_changed()\0validate_and_accept()\0"
};

const QMetaObject Haruhi::SessionPrivate::SettingsDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Haruhi__SessionPrivate__SettingsDialog,
      qt_meta_data_Haruhi__SessionPrivate__SettingsDialog, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::SessionPrivate::SettingsDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::SessionPrivate::SettingsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::SessionPrivate::SettingsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__SessionPrivate__SettingsDialog))
        return static_cast<void*>(const_cast< SettingsDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int Haruhi::SessionPrivate::SettingsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: state_changed(); break;
        case 1: validate_and_accept(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_Haruhi__Session[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x08,
      34,   16,   16,   16, 0x08,
      49,   16,   16,   16, 0x08,
      67,   16,   16,   16, 0x08,
      84,   16,   16,   16, 0x08,
     109,   16,   16,   16, 0x08,
     132,   16,   16,   16, 0x08,
     151,   16,   16,   16, 0x08,
     172,   16,   16,   16, 0x08,
     193,   16,   16,   16, 0x08,
     215,   16,   16,   16, 0x08,
     237,   16,   16,   16, 0x08,
     252,   16,   16,   16, 0x08,
     268,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Haruhi__Session[] = {
    "Haruhi::Session\0\0session_loader()\0"
    "save_session()\0save_session_as()\0"
    "rename_session()\0tempo_value_changed(int)\0"
    "panic_button_clicked()\0create_main_menu()\0"
    "stop_audio_backend()\0stop_event_backend()\0"
    "start_audio_backend()\0start_event_backend()\0"
    "show_program()\0show_backends()\0"
    "reconnect_to_jack()\0"
};

const QMetaObject Haruhi::Session::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Haruhi__Session,
      qt_meta_data_Haruhi__Session, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Haruhi::Session::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Haruhi::Session::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Haruhi::Session::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Haruhi__Session))
        return static_cast<void*>(const_cast< Session*>(this));
    return QWidget::qt_metacast(_clname);
}

int Haruhi::Session::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: session_loader(); break;
        case 1: save_session(); break;
        case 2: save_session_as(); break;
        case 3: rename_session(); break;
        case 4: tempo_value_changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: panic_button_clicked(); break;
        case 6: create_main_menu(); break;
        case 7: stop_audio_backend(); break;
        case 8: stop_event_backend(); break;
        case 9: start_audio_backend(); break;
        case 10: start_event_backend(); break;
        case 11: show_program(); break;
        case 12: show_backends(); break;
        case 13: reconnect_to_jack(); break;
        default: ;
        }
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
