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
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/core/graph.h>
#include <haruhi/components/audio_backend/audio_backend.h>
#include <haruhi/components/event_backend/backend.h>
#include <haruhi/widgets/clickable_label.h>
#include <haruhi/utility/numeric.h>
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


Private::Global::Global (Session* session, QWidget* parent):
	QWidget (parent),
	_session (session),
	_loading_params (false)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_tuning_hz = new QLabel (this);

	_tuning = new QSpinBox (this);
	_tuning->setRange (-50, +50);
	_tuning->setSuffix (" cents");
	_tuning->setValue (0);
	QObject::connect (_tuning, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	_transpose = new QSpinBox (this);
	_transpose->setRange (-60, 60);
	_transpose->setSuffix (" semitones");
	_transpose->setValue (0);
	QObject::connect (_transpose, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	_engine_thread_priority = new QSpinBox (this);
	_engine_thread_priority->setRange (1, 99);
	_engine_thread_priority->setValue (50);
	QToolTip::add (_engine_thread_priority, "Higher values mean higher priority");
	QObject::connect (_engine_thread_priority, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	_level_meter_fps = new QSpinBox (this);
	_level_meter_fps->setRange (10, 50);
	_level_meter_fps->setValue (30);
	QObject::connect (_level_meter_fps, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	QGridLayout* group_layout = new QGridLayout (this);
	group_layout->addWidget (new QLabel ("Master tuning:", this), 0, 0);
	group_layout->addWidget (_tuning, 0, 1);
	group_layout->addWidget (_tuning_hz, 0, 2);
	group_layout->addWidget (new QLabel ("Transpose:", this), 1, 0);
	group_layout->addWidget (_transpose, 1, 1);
	group_layout->addWidget (new QLabel ("Engine thread priority:", this), 2, 0);
	group_layout->addWidget (_engine_thread_priority, 2, 1);
	group_layout->addWidget (new QLabel ("Level Meter FPS:", this), 3, 0);
	group_layout->addWidget (_level_meter_fps, 3, 1);
	group_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 3);
	group_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 4, 0);

	update_widgets();
}


void
Private::Global::load_params()
{
	_loading_params = true;
	_tuning->setValue (_session->parameters().tuning);
	_transpose->setValue (_session->parameters().transpose);
	_engine_thread_priority->setValue (_session->parameters().engine_thread_priority);
	_level_meter_fps->setValue (_session->parameters().level_meter_fps);
	_loading_params = false;
	update_widgets();
}


void
Private::Global::update_params()
{
	if (_loading_params)
		return;
	_session->parameters().tuning = _tuning->value();
	_session->parameters().transpose = _transpose->value();
	_session->parameters().engine_thread_priority = _engine_thread_priority->value();
	_session->parameters().level_meter_fps = _level_meter_fps->value();
	update_widgets();
	_session->apply_parameters();
}


void
Private::Global::update_widgets()
{
	_tuning_hz->setText (QString::number (_session->master_tune(), 'f', 2) + "Hz");
}


Session::Parameters::Parameters():
	tuning (0),
	transpose (0),
	engine_thread_priority (50),
	level_meter_fps (30),
	tempo (120.0)
{
	limit_values();
}


void
Session::Parameters::load_state (QDomElement const& element)
{
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "tuning")
				tuning = e.text().toInt();
			else if (e.tagName() == "transpose")
				transpose = e.text().toInt();
			else if (e.tagName() == "engine-thread-priority")
				engine_thread_priority = e.text().toInt();
			else if (e.tagName() == "level-meter-fps")
				level_meter_fps = e.text().toInt();
			else if (e.tagName() == "tempo")
				tempo = e.text().toFloat();
		}
	}
	limit_values();
}


void
Session::Parameters::limit_values()
{
	limit_value (tuning, -50, 50);
	limit_value (transpose, -60, 60);
	limit_value (engine_thread_priority, 1, 99);
	limit_value (level_meter_fps, 10, 50);
	limit_value (tempo, 20.0f, 400.0f);
}


void
Session::Parameters::save_state (QDomElement& element) const
{
	QDomElement par_tuning = element.ownerDocument().createElement ("tuning");
	par_tuning.appendChild (element.ownerDocument().createTextNode (QString::number (tuning)));

	QDomElement par_transpose = element.ownerDocument().createElement ("transpose");
	par_transpose.appendChild (element.ownerDocument().createTextNode (QString::number (transpose)));

	QDomElement par_engine_thread_priority = element.ownerDocument().createElement ("engine-thread-priority");
	par_engine_thread_priority.appendChild (element.ownerDocument().createTextNode (QString::number (engine_thread_priority)));

	QDomElement par_level_meter_fps = element.ownerDocument().createElement ("level-meter-fps");
	par_level_meter_fps.appendChild (element.ownerDocument().createTextNode (QString::number (level_meter_fps)));

	QDomElement par_tempo = element.ownerDocument().createElement ("tempo");
	par_tempo.appendChild (element.ownerDocument().createTextNode (QString::number (tempo)));

	element.appendChild (par_tuning);
	element.appendChild (par_transpose);
	element.appendChild (par_engine_thread_priority);
	element.appendChild (par_level_meter_fps);
	element.appendChild (par_tempo);
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

		_tempo_spinbox = new QDoubleSpinBox (inner_header);
		_tempo_spinbox->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		_tempo_spinbox->setRange (20.0, 400.0);
		_tempo_spinbox->setDecimals (2);
		_tempo_spinbox->setWrapping (true);
		_tempo_spinbox->setValue (120.0);
		QToolTip::add (_tempo_spinbox, "Master tempo");
		QObject::connect (_tempo_spinbox, SIGNAL (valueChanged (double)), this, SLOT (tempo_value_changed (double)));

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

		_global = new Private::Global (this, _backends);

		_audio_tab = create_container (this);
		_event_tab = create_container (this);
		_devices_manager_tab = new DevicesManager::Panel (this);

		// Add tabs:
		_backends->addTab (_global, Config::Icons22::configure(), "Global");
		_backends->addTab (_audio_tab, Config::Icons22::show_audio(), "Audio backend");
		_backends->addTab (_event_tab, Config::Icons22::show_event(), "Input devices");
		_backends->addTab (_devices_manager_tab, Config::Icons22::show_event(), "Devices manager");

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
	delete _program;

	stop_audio_backend();
	stop_event_backend();

	delete _engine;
	delete _graph;
}


void
Session::apply_parameters()
{
	graph()->set_master_tune (master_tune());
	engine()->set_sched (Thread::SchedRR, _parameters.engine_thread_priority);
	meter_panel()->level_meters_group()->set_fps (_parameters.level_meter_fps);
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

	// Save parameters:
	QDomElement parameters = element.ownerDocument().createElement ("parameters");
	_parameters.save_state (parameters);

	// Save audio-backend:
	QDomElement audio_backend = element.ownerDocument().createElement ("audio-backend");
	_audio_backend->save_state (audio_backend);
	audio_backend.setAttribute ("id", this->audio_backend()->id());

	// Save event-backend:
	QDomElement event_backend = element.ownerDocument().createElement ("event-backend");
	_event_backend->save_state (event_backend);
	event_backend.setAttribute ("id", this->event_backend()->id());

	// Save programs:
	QDomElement program = element.ownerDocument().createElement ("program");
	_program->save_state (program);

	element.appendChild (parameters);
	element.appendChild (audio_backend);
	element.appendChild (event_backend);
	element.appendChild (program);
}


void
Session::load_state (QDomElement const& element)
{
	set_name (element.attribute ("name", "").ascii());

	QDomElement parameters_element;
	QDomElement audio_backend_element;
	QDomElement event_backend_element;
	QDomElement program_element;

	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "parameters")
				parameters_element = e;
			else if (e.tagName() == "audio-backend")
				audio_backend_element = e;
			else if (e.tagName() == "event-backend")
				event_backend_element = e;
			else if (e.tagName() == "program")
				program_element = e;
		}
	}

	if (!parameters_element.isNull())
	{
		parameters().load_state (parameters_element);
		_global->load_params();
		apply_parameters();
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


float
Session::master_tune() const
{
	return 440.0f * std::pow (2.0f, (1.0f / 12.0f) * (_parameters.tuning / 100.0f + _parameters.transpose));
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
Session::tempo_value_changed (double new_tempo)
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
		delete _event_backend;
	}
	_event_backend = 0;
}


void
Session::start_audio_backend()
{
	try {
		_audio_backend = new AudioBackend::Backend (this, "Haruhi", 1, _audio_tab);
		_audio_backend->show();
		_audio_backend->connect();
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
		_event_backend = new EventBackend::Backend (this, "Haruhi", 2, _event_tab);
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


QWidget*
Session::create_container (QWidget* parent)
{
	// Configure layouts for audio and event tabs:
	QWidget* w = new QWidget (parent);
	w->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QHBoxLayout* layout = new QHBoxLayout (w, 0, 0);
	layout->setAutoAdd (true);
	return w;
}

} // namespace Haruhi

