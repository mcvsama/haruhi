/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QFile>
#include <QtCore/QObjectList>
#include <QtCore/QTextStream>
#include <QtGui/QApplication>
#include <QtGui/QShortcut>
#include <QtGui/QCursor>
#include <QtGui/QToolTip>
#include <QtGui/QMessageBox>
#include <QtGui/QWhatsThis>
#include <QtGui/QSlider>
#include <QtGui/QFileDialog>
#include <QtGui/QCloseEvent>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/core/graph.h>
#include <haruhi/components/audio_backend/audio_backend.h>
#include <haruhi/components/event_backend/event_backend.h>
#include <haruhi/widgets/clickable_label.h>
#include <haruhi/periodic_updater.h>

// Local:
#include "session.h"


namespace Haruhi {

namespace Private = SessionPrivate;

Private::SettingsDialog::SettingsDialog (QWidget* parent, Session* session):
	QDialog (parent),
	_session (session)
{
	setCaption ("Session properties");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::dialog_margin, Config::spacing);
	layout->setResizeMode (QLayout::FreeResize);

		QHBoxLayout* name_layout = new QHBoxLayout (layout, Config::spacing);

			QLabel* name_label = new QLabel ("Session name:", this);

			_name = new QLineEdit (_session->_name, this);
			_name->selectAll();
			_name->setFocus();

		name_layout->addWidget (name_label);
		name_layout->addWidget (_name);

		QHBoxLayout* buttons_layout = new QHBoxLayout (layout, Config::spacing);

			_accept_button = new QPushButton ("&Ok", this);
			_accept_button->setDefault (true);

			_reject_button = new QPushButton ("&Cancel", this);

		buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
		buttons_layout->addWidget (_accept_button);
		buttons_layout->addWidget (_reject_button);

	QObject::connect (_name, SIGNAL (textChanged (const QString&)), this, SLOT (state_changed()));
	QObject::connect (_accept_button, SIGNAL (clicked()), this, SLOT (validate_and_accept()));
	QObject::connect (_reject_button, SIGNAL (clicked()), this, SLOT (reject()));

	state_changed();

	adjustSize();
	setMinimumHeight (height());
	setMaximumHeight (height());
}


QString
Private::SettingsDialog::name() const
{
	return _name->text();
}


void
Private::SettingsDialog::apply()
{
	_session->set_name (_name->text());
}


void
Private::SettingsDialog::state_changed()
{
	_accept_button->setEnabled (!_name->text().isEmpty());
}


void
Private::SettingsDialog::validate_and_accept()
{
	if (_name->text().isEmpty())
		QMessageBox::warning (this, "Session name", "Enter name for the session.");
	else
		accept();
}


Session::MeterPanel::MeterPanel (Session* session, QWidget* parent):
	QFrame (parent),
	_session (session)
{
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
	setFrameStyle (QFrame::StyledPanel | QFrame::Raised);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::margin, Config::spacing);

		_level_meters_group = new LevelMetersGroup (this);
		_master_volume = new DialControl (this, MinVolume, MaxVolume, 0.75 * ZeroVolume);
		QToolTip::add (_master_volume, "Master Volume");

		layout->addWidget (_level_meters_group);
		layout->addWidget (_master_volume);
}


Session::AudioTab::AudioTab (Session* session, QWidget* parent):
	QFrame (parent),
	_session (session)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	QVBoxLayout* layout = new QVBoxLayout (this, 0, 0);

	_backend_parent = new QWidget (this);
	_backend_parent->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	QVBoxLayout* backend_parent_layout = new QVBoxLayout (_backend_parent, 0, 0);
	backend_parent_layout->setAutoAdd (true);

	layout->addWidget (_backend_parent);
}


Session::EventTab::EventTab (Session* session, QWidget* parent):
	QFrame (parent),
	_session (session)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	QVBoxLayout* layout = new QVBoxLayout (this, 0, 0);

	_backend_parent = new QWidget (this);
	_backend_parent->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	QVBoxLayout* backend_parent_layout = new QVBoxLayout (_backend_parent, 0, 0);
	backend_parent_layout->setAutoAdd (true);

	layout->addWidget (_backend_parent);
}


Session::Session (QWidget* parent):
	QWidget (parent),
	_graph (new Core::Graph()),
	_audio_backend (0),
	_event_backend (0),
	_unit_loader (new UnitLoader (this))
{
	_name = "";

	setCaption ("Haruhi");
	setIcon (QPixmap ("share/images/haruhi.png"));
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	create_main_menu();

	_layout = new QVBoxLayout (this, Config::window_margin, Config::spacing);

		QFrame* header = new QFrame (this);
		header->setAutoFillBackground (true);
		header->setBackgroundColor (QColor (0xda, 0xe1, 0xe9));
		header->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
		header->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

		QHBoxLayout* header_layout = new QHBoxLayout (header, Config::margin);

		QWidget* inner_header = new QWidget (header);
		inner_header->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

		_session_name = new ClickableLabel (_name, inner_header);
		QFont f (QApplication::font());
		f.setPointSize (15);
		f.setWeight (QFont::Normal);
		_session_name->setFont (f);
		_session_name->setCursor (QCursor (Qt::PointingHandCursor));
		QToolTip::add (_session_name, "Click to rename session");
		QObject::connect (_session_name, SIGNAL (clicked()), this, SLOT (rename_session()));

		QLabel* tempo_note = new QLabel (QString::fromUtf8 ("♩ = "), inner_header);
		f.setPointSize (16);
		f.setWeight (QFont::Normal);
		tempo_note->setFont (f);
		tempo_note->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

		_tempo_spinbox = new QSpinBox (20, 400, 1, inner_header);
		_tempo_spinbox->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		_tempo_spinbox->setWrapping (true);
		_tempo_spinbox->setValue (120);
		QToolTip::add (_tempo_spinbox, "Master tempo");
		QObject::connect (_tempo_spinbox, SIGNAL (valueChanged (int)), this, SLOT (tempo_value_changed (int)));

		_panic_button = new QPushButton ("Panic!", inner_header);
		_panic_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		QToolTip::add (_panic_button, "Stops all sound processing (F10)");
		QObject::connect (_panic_button, SIGNAL (clicked()), this, SLOT (panic_button_clicked()));
		new QShortcut (Qt::Key_F10, this, SLOT (panic_button_clicked()));

		_main_menu_button = new QPushButton ("Menu", inner_header);
		_main_menu_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		_main_menu_button->setPopup (_main_menu);

		QHBoxLayout* inner_header_layout = new QHBoxLayout (inner_header, 0, Config::spacing);
		inner_header_layout->addWidget (_session_name);
		inner_header_layout->addItem (new QSpacerItem (0, 0));
		inner_header_layout->addWidget (tempo_note);
		inner_header_layout->addWidget (_tempo_spinbox);
		inner_header_layout->addItem (new QSpacerItem (2 * Config::spacing, 0, QSizePolicy::Maximum, QSizePolicy::Minimum));
		inner_header_layout->addWidget (_panic_button);
		inner_header_layout->addWidget (_main_menu_button);

		header_layout->addWidget (inner_header);

	_layout->addWidget (header);

		QHBoxLayout* bottom_layout = new QHBoxLayout (_layout, Config::spacing + 1);

		_meter_panel = new MeterPanel (this, this);
		_stack = new QStackedWidget (this);
		_backends = new QTabWidget (this);
		_backends->setTabPosition (QTabWidget::South);
		_backends->setIconSize (QSize (32, 22));

		_audio = new AudioTab (this, _backends);
		_event = new EventTab (this, _backends);

		_backends->addTab (_audio, Config::Icons22::show_audio(), "Audio");
		_backends->addTab (_event, Config::Icons22::show_event(), "Event");

		// Start engine and backends before program is loaded:
		_engine = new Engine (this);
		start_event_backend();
		start_audio_backend();
		_engine->start();

		_program = new Program (this, _stack);

		_stack->addWidget (_program);
		_stack->addWidget (_backends);

		_stack->setCurrentWidget (_program);

		bottom_layout->addWidget (_meter_panel);
		bottom_layout->addWidget (_stack);
}


Session::~Session()
{
	delete _unit_loader;

	_program->reparent (0, QPoint (0, 0), false);
	delete _program;

	stop_audio_backend();
	stop_event_backend();

	delete _engine;
	delete _graph;
}


void
Session::load_session (QString const& file_name)
{
	try {
		QDomDocument document;

		// Open file:
		QFile file (file_name);
		if (!file.open (IO_ReadOnly))
			throw Exception (QString ("Could not open session file: ") + file.errorString());
		else if (!document.setContent (&file, true))
			throw Exception ("Failed to parse session file.");

		// Process file:
		if (document.documentElement().tagName() == "haruhi-session")
			load_state (document.documentElement());
		else
			throw Exception ("Failed to process session file: invalid XML root element.");

		_file_name = file_name;

		// Add session to recent sessions list:
		Config::recent_sessions().push_back (Config::RecentSession (_name, file_name, ::time (0)));
		Config::update_recent_sessions();
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Error while loading session", e.what());
	}
}


void
Session::save_session (QString const& file_name)
{
	try {
		QDomDocument document;
		QDomElement root = document.createElement ("haruhi-session");
		document.appendChild (root);
		save_state (root);

		// TODO maybe saving should be done in another thread? std::future?
		QFile file (file_name + "~");
		if (!file.open (IO_WriteOnly))
			throw Exception (QString ("Could not save session file: ") + file.errorString());
		QTextStream ts (&file);
		ts << document.toString();
		file.flush();
		file.close();
		::rename ((file_name + "~").toUtf8(), file_name.toUtf8());

		// Add session to recent sessions list:
		Config::recent_sessions().push_back (Config::RecentSession (_name, file_name, ::time (0)));
		Config::update_recent_sessions();
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Error while loading session", e.what());
	}
}


void
Session::save_state (QDomElement& element) const
{
	element.setAttribute ("name", name());

	// Save audio-backend:
	QDomElement audio_backend = element.ownerDocument().createElement ("audio-backend");
	this->audio_backend()->save_state (audio_backend);
	audio_backend.setAttribute ("id", this->audio_backend()->id());

	// Save event-backend:
	QDomElement event_backend = element.ownerDocument().createElement ("event-backend");
	this->event_backend()->save_state (event_backend);
	event_backend.setAttribute ("id", this->event_backend()->id());

	// Save programs:
	QDomElement program = element.ownerDocument().createElement ("program");
	_program->save_state (program);

	element.appendChild (audio_backend);
	element.appendChild (event_backend);
	element.appendChild (program);
}


void
Session::load_state (QDomElement const& element)
{
	set_name (element.attribute ("name", "").ascii());

	QDomElement audio_backend_element;
	QDomElement event_backend_element;
	QDomElement program_element;

	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "audio-backend")
				audio_backend_element = e;
			else if (e.tagName() == "event-backend")
				event_backend_element = e;
			else if (e.tagName() == "program")
				program_element = e;
		}
	}

	if (!audio_backend_element.isNull() && !event_backend_element.isNull() && !program_element.isNull())
	{
		// Components must be restored in given order (backends must get their
		// IDs before program and connections between units are restored):

		audio_backend()->load_state (audio_backend_element);
		audio_backend()->set_id (audio_backend_element.attribute ("id").toInt());

		event_backend()->load_state (event_backend_element);
		event_backend()->set_id (event_backend_element.attribute ("id").toInt());

		_program->load_state (program_element);
	}
	else
		QMessageBox::warning (this, "Error while loading session", "Could not load session due to missing information in session file.");
}


void
Session::session_loader()
{
	Haruhi::haruhi()->session_loader();	
}


void
Session::save_session()
{
	if (_file_name.isEmpty())
		save_session_as();
	else
		save_session (_file_name);
}


void
Session::save_session_as()
{
	QFileDialog* file_dialog = new QFileDialog (this, "Save session", ".", QString());
	file_dialog->setNameFilter ("All Haruhi session files (*.haruhi-session)");
	file_dialog->setMode (QFileDialog::AnyFile);
	if (file_dialog->exec() == QFileDialog::Accepted)
	{
		_file_name = file_dialog->selectedFile().ascii();
		if (!QString (_file_name).endsWith (".haruhi-session", false))
			_file_name += ".haruhi-session";
		save_session (_file_name);
	}
}


void
Session::rename_session()
{
	Private::SettingsDialog* dialog = new Private::SettingsDialog (this, this);
	if (dialog->exec() == Private::SettingsDialog::Accepted)
		dialog->apply();
}


void
Session::tempo_value_changed (int new_tempo)
{
	graph()->lock();
	graph()->set_tempo (new_tempo);
	graph()->unlock();
}


void
Session::panic_button_clicked()
{
	graph()->lock();
	graph()->panic();
	graph()->unlock();
}


void
Session::create_main_menu()
{
	_main_menu = new QMenu (this);

	_main_menu->addAction (Config::Icons16::show_program(), "Show &program", this, SLOT (show_program()), Qt::Key_F1);
	_main_menu->addAction (Config::Icons16::show_backends(), "Show &backends", this, SLOT (show_backends()), Qt::Key_F2);
	_main_menu->addSeparator();
	_main_menu->addAction (Config::Icons16::session_manager(), "Session &manager…", this, SLOT (session_loader()), Qt::CTRL + Qt::Key_M);
	_main_menu->addAction (Config::Icons16::save(), "&Save", this, SLOT (save_session()), Qt::CTRL + Qt::Key_S);
	_main_menu->addAction (Config::Icons16::save_as(), "Sa&ve as…", this, SLOT (save_session_as()), Qt::CTRL + Qt::SHIFT + Qt::Key_S);
	_main_menu->addSeparator();
	_main_menu->addAction (Config::Icons16::disconnect(), "&Reconnect to JACK", this, SLOT (reconnect_to_jack()), Qt::CTRL + Qt::Key_J);
	_main_menu->addSeparator();
	_main_menu->addAction (Config::Icons16::exit(), "&Quit", Haruhi::haruhi()->application(), SLOT (quit()), Qt::CTRL + Qt::Key_Q);
}


void
Session::stop_audio_backend()
{
	if (_audio_backend)
	{
		_audio_backend->disable();
		_audio_backend->disconnect();
		_audio_backend->hide();
		_audio_backend->reparent (0, QPoint (0, 0), false);
		delete _audio_backend;
	}
	_audio_backend = 0;
}


void
Session::stop_event_backend()
{
	if (_event_backend)
	{
		_event_backend->disable();
		_event_backend->disconnect();
		_event_backend->hide();
		_event_backend->reparent (0, QPoint (0, 0), false);
		delete _event_backend;
	}
	_event_backend = 0;
}


void
Session::start_audio_backend()
{
	try {
		_audio_backend = new AudioBackend (this, "Haruhi", 1, _audio->_backend_parent);
		_audio_backend->show();
		_event_backend->connect();
		_audio_backend->enable();
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Could not create audio backend", QString ("Could not start audio backend: ") + e.what());
	}
}


void
Session::start_event_backend()
{
	try {
		_event_backend = new EventBackend (this, "Haruhi", 2, _event->_backend_parent);
		_event_backend->show();
		_event_backend->connect();
		_event_backend->enable();
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Could not create event backend", QString ("Could not start event backend: ") + e.what());
	}
}


void
Session::closeEvent (QCloseEvent* e)
{
	e->accept();
	Haruhi::haruhi()->ok_to_quit();
}

} // namespace Haruhi

