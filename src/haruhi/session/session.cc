/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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
#include <typeinfo>

// Qt:
#include <QtCore/QFile>
#include <QtCore/QObjectList>
#include <QtCore/QTextStream>
#include <QtGui/QApplication>
#include <QtGui/QShortcut>
#include <QtGui/QCursor>
#include <QtGui/QMessageBox>
#include <QtGui/QWhatsThis>
#include <QtGui/QSlider>
#include <QtGui/QFileDialog>
#include <QtGui/QCloseEvent>
#include <QtGui/QGridLayout>
#include <QtGui/QLayout>
#include <QtGui/QTextDocument>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/graph.h>
#include <haruhi/application/services.h>
#include <haruhi/components/audio_backend/backend.h>
#include <haruhi/components/event_backend/backend.h>
#include <haruhi/session/periodic_updater.h>
#include <haruhi/settings/haruhi_settings.h>
#include <haruhi/settings/session_loader_settings.h>
#include <haruhi/widgets/clickable_label.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/qdom.h>

// Local:
#include "session.h"


namespace Haruhi {

namespace Private = SessionPrivate;

Private::SettingsDialog::SettingsDialog (QWidget* parent, Session* session):
	QDialog (parent),
	_session (session)
{
	setWindowTitle ("Session properties");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	QLabel* name_label = new QLabel ("Session name:", this);

	_name = new QLineEdit (_session->_name, this);
	_name->selectAll();
	_name->setFocus();
	QObject::connect (_name, SIGNAL (textChanged (const QString&)), this, SLOT (state_changed()));

	_accept_button = new QPushButton ("&Ok", this);
	_accept_button->setDefault (true);
	QObject::connect (_accept_button, SIGNAL (clicked()), this, SLOT (validate_and_accept()));

	_reject_button = new QPushButton ("&Cancel", this);
	QObject::connect (_reject_button, SIGNAL (clicked()), this, SLOT (reject()));

	// Layouts:

	QHBoxLayout* name_layout = new QHBoxLayout();
	name_layout->setSpacing (Config::spacing());
	name_layout->addWidget (name_label);
	name_layout->addWidget (_name);

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::spacing());
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (_accept_button);
	buttons_layout->addWidget (_reject_button);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::dialog_margin());
	layout->setSpacing (Config::spacing());
	layout->addLayout (name_layout);
	layout->addLayout (buttons_layout);

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


Private::SessionGlobal::SessionGlobal (Session* session, QWidget* parent):
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

	QGridLayout* group_layout = new QGridLayout (this);
	group_layout->addWidget (new QLabel ("Master tuning:", this), 0, 0);
	group_layout->addWidget (_tuning, 0, 1);
	group_layout->addWidget (_tuning_hz, 0, 2);
	group_layout->addWidget (new QLabel ("Transpose:", this), 1, 0);
	group_layout->addWidget (_transpose, 1, 1);
	group_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 3);
	group_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 2, 0);

	update_widgets();
}


void
Private::SessionGlobal::load_params()
{
	_loading_params = true;
	_tuning->setValue (_session->parameters().tuning);
	_transpose->setValue (_session->parameters().transpose);
	_loading_params = false;
	update_widgets();
}


void
Private::SessionGlobal::update_params()
{
	if (_loading_params)
		return;
	_session->parameters().tuning = _tuning->value();
	_session->parameters().transpose = _transpose->value();
	update_widgets();
	_session->apply_parameters();
}


void
Private::SessionGlobal::update_widgets()
{
	_tuning_hz->setText (QString::number (_session->master_tune().Hz(), 'f', 2) + " Hz");
}


Private::HaruhiGlobal::HaruhiGlobal (Session* session, QWidget* parent):
	QWidget (parent),
	_session (session),
	_loading_params (false)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_engine_thread_priority = new QSpinBox (this);
	_engine_thread_priority->setRange (1, 99);
	_engine_thread_priority->setValue (50);
	_engine_thread_priority->setToolTip ("Higher values mean higher priority");
	QObject::connect (_engine_thread_priority, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	_level_meter_fps = new QSpinBox (this);
	_level_meter_fps->setRange (10, 50);
	_level_meter_fps->setValue (30);
	QObject::connect (_level_meter_fps, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	QGridLayout* group_layout = new QGridLayout (this);
	group_layout->addWidget (new QLabel ("Engine thread priority:", this), 0, 0);
	group_layout->addWidget (_engine_thread_priority, 0, 1);
	group_layout->addWidget (new QLabel ("Level Meter FPS:", this), 1, 0);
	group_layout->addWidget (_level_meter_fps, 1, 1);
	group_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 2);
	group_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 2, 0);

	update_widgets();
}


void
Private::HaruhiGlobal::load_params()
{
	HaruhiSettings* haruhi_settings = Haruhi::haruhi()->haruhi_settings();

	_loading_params = true;
	_engine_thread_priority->setValue (haruhi_settings->engine_thread_priority());
	_level_meter_fps->setValue (haruhi_settings->level_meter_fps());
	_loading_params = false;
	update_widgets();
}


void
Private::HaruhiGlobal::update_params()
{
	if (_loading_params)
		return;

	HaruhiSettings* haruhi_settings = Haruhi::haruhi()->haruhi_settings();

	haruhi_settings->set_engine_thread_priority (_engine_thread_priority->value());
	haruhi_settings->set_level_meter_fps (_level_meter_fps->value());
	haruhi_settings->save();
	update_widgets();
	_session->apply_parameters();
}


void
Private::HaruhiGlobal::update_widgets()
{
}


Session::Parameters::Parameters():
	tuning (0),
	transpose (0),
	tempo (120.0),
	master_volume (0)
{
	limit_values();
}


void
Session::Parameters::load_state (QDomElement const& element)
{
	for (QDomElement& e: element)
	{
		if (e.tagName() == "tuning")
			tuning = e.text().toInt();
		else if (e.tagName() == "transpose")
			transpose = e.text().toInt();
		else if (e.tagName() == "tempo")
			tempo = e.text().toFloat();
		else if (e.tagName() == "master-volume")
			master_volume = e.text().toInt();
	}
	limit_values();
}


void
Session::Parameters::save_state (QDomElement& element) const
{
	QDomElement par_tuning = element.ownerDocument().createElement ("tuning");
	par_tuning.appendChild (element.ownerDocument().createTextNode (QString::number (tuning)));

	QDomElement par_transpose = element.ownerDocument().createElement ("transpose");
	par_transpose.appendChild (element.ownerDocument().createTextNode (QString::number (transpose)));

	QDomElement par_tempo = element.ownerDocument().createElement ("tempo");
	par_tempo.appendChild (element.ownerDocument().createTextNode (QString::number (tempo)));

	QDomElement par_master_volume = element.ownerDocument().createElement ("master-volume");
	par_master_volume.appendChild (element.ownerDocument().createTextNode (QString::number (master_volume)));

	element.appendChild (par_tuning);
	element.appendChild (par_transpose);
	element.appendChild (par_tempo);
	element.appendChild (par_master_volume);
}


void
Session::Parameters::limit_values()
{
	limit_value (tuning, -50, 50);
	limit_value (transpose, -60, 60);
	limit_value (tempo, 20.0f, 400.0f);
	limit_value (master_volume, static_cast<int> (MeterPanel::MinVolume), static_cast<int> (MeterPanel::MaxVolume));
}


Session::MeterPanel::MeterPanel (Session* session, QWidget* parent):
	QFrame (parent),
	_session (session)
{
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
	setFrameStyle (QFrame::StyledPanel | QFrame::Raised);

	_level_meters_group = new LevelMetersGroup (this);
	_master_volume = new DialControl (this,
									  { MinVolume, MaxVolume },
									  ZeroVolume * std::pow (attenuate_db (-3.0f), 1.0f / M_E));
	_master_volume->setToolTip ("Master Volume");
	QObject::connect (_master_volume, SIGNAL (valueChanged (int)), _session, SLOT (master_volume_changed (int)));

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::margin());
	layout->setSpacing (Config::spacing());
	layout->addWidget (_level_meters_group);
	layout->addWidget (_master_volume);
}


Session::Session (QWidget* parent):
	QWidget (parent),
	_graph (new Graph()),
	_audio_backend (0),
	_event_backend (0),
	_engine (0),
	_plugin_loader (new PluginLoader()),
	_devices_manager (0)
{
	_name = "";

	setWindowIcon (QPixmap ("share/images/haruhi.png"));
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	create_main_menu();

	QFrame* header = new QFrame (this);
	header->setAutoFillBackground (true);
	QPalette p = header->palette();
	p.setColor (QPalette::Window, QColor (0xda, 0xe1, 0xe9));
	header->setPalette (p);
	header->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	header->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	QWidget* inner_header = new QWidget (header);
	inner_header->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	_session_name = new ClickableLabel (_name, inner_header);
	QFont f (QApplication::font());
	f.setPointSize (15);
	f.setWeight (QFont::Normal);
	_session_name->setFont (f);
	_session_name->setCursor (QCursor (Qt::PointingHandCursor));
	_session_name->setTextFormat (Qt::PlainText);
	_session_name->setToolTip ("Click to rename session");
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
	_tempo_spinbox->setToolTip ("Master tempo");
	QObject::connect (_tempo_spinbox, SIGNAL (valueChanged (double)), this, SLOT (tempo_value_changed (double)));

	_panic_button = new QPushButton ("Panic!", inner_header);
	_panic_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_panic_button->setToolTip ("Stops all sound processing (F10)");
	QObject::connect (_panic_button, SIGNAL (clicked()), this, SLOT (panic_button_clicked()));
	new QShortcut (Qt::Key_F10, this, SLOT (panic_button_clicked()));
	new QShortcut (Qt::ControlModifier + Qt::Key_PageUp, this, SLOT (show_prev_plugin()));
	new QShortcut (Qt::ControlModifier + Qt::Key_PageDown, this, SLOT (show_next_plugin()));

	_main_menu_button = new QPushButton ("Menu", inner_header);
	_main_menu_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_main_menu_button->setMenu (_main_menu);

	_meter_panel = new MeterPanel (this, this);
	_stack = new QStackedWidget (this);

	_session_settings = new QTabWidget (this);
	_session_settings->setTabPosition (QTabWidget::South);
	_session_settings->setIconSize (QSize (32, 22));

	_haruhi_settings = new QTabWidget (this);
	_haruhi_settings->setTabPosition (QTabWidget::South);
	_haruhi_settings->setIconSize (QSize (32, 22));

	_session_global = new Private::SessionGlobal (this, _session_settings);
	_haruhi_global = new Private::HaruhiGlobal (this, _session_settings);
	_audio_tab = create_container (this);
	_event_tab = create_container (this);

	_devices_manager = new DevicesManager::Panel (this, Haruhi::haruhi()->devices_manager_settings());

	// Layouts:

	QHBoxLayout* header_layout = new QHBoxLayout (header);
	header_layout->setMargin (Config::margin());
	header_layout->addWidget (inner_header);

	QHBoxLayout* inner_header_layout = new QHBoxLayout (inner_header);
	inner_header_layout->setMargin (0);
	inner_header_layout->setSpacing (Config::spacing());
	inner_header_layout->addWidget (_session_name);
	inner_header_layout->addItem (new QSpacerItem (0, 0));
	inner_header_layout->addWidget (tempo_note);
	inner_header_layout->addWidget (_tempo_spinbox);
	inner_header_layout->addItem (new QSpacerItem (2 * Config::spacing(), 0, QSizePolicy::Maximum, QSizePolicy::Minimum));
	inner_header_layout->addWidget (_panic_button);
	inner_header_layout->addWidget (_main_menu_button);

	QHBoxLayout* bottom_layout = new QHBoxLayout();
	bottom_layout->setSpacing (Config::spacing() + 1);
	bottom_layout->addWidget (_meter_panel);
	bottom_layout->addWidget (_stack);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::window_margin());
	layout->setSpacing (Config::spacing());
	layout->addWidget (header);
	layout->addLayout (bottom_layout);

	// Add tabs:
	_session_settings->addTab (_session_global, Resources::Icons22::configure(), "Session settings");
	_session_settings->addTab (_audio_tab, Resources::Icons22::show_audio(), "Audio backend");
	_session_settings->addTab (_event_tab, Resources::Icons22::show_event(), "Input devices");

	_haruhi_settings->addTab (_haruhi_global, Resources::Icons22::configure(), "Haruhi settings");
	_haruhi_settings->addTab (_devices_manager, Resources::Icons22::show_event(), "Devices manager");

	// Start engine and backends before program is loaded:
	_engine = new Engine (this);
	start_event_backend();
	start_audio_backend();
	_engine->start();

	_program = new Program (this, _stack);

	_stack->addWidget (_program);
	_stack->addWidget (_session_settings);
	_stack->addWidget (_haruhi_settings);

	_stack->setCurrentWidget (_program);

	update_window_title();
}


Session::~Session()
{
	// In this order:
	delete _program;
	delete _plugin_loader;

	delete _engine;
	stop_audio_backend();
	stop_event_backend();
	delete _graph;
}


void
Session::apply_parameters()
{
	HaruhiSettings* haruhi_settings = Haruhi::haruhi()->haruhi_settings();

	graph()->set_master_tune (master_tune());
	int prio = haruhi_settings->engine_thread_priority();
	engine()->set_sched (Thread::SchedFIFO, prio);
	Services::hi_priority_work_performer()->set_sched (Thread::SchedFIFO, prio);
	Services::lo_priority_work_performer()->set_sched (Thread::SchedOther, 0);
	meter_panel()->level_meters_group()->set_fps (haruhi_settings->level_meter_fps());
	meter_panel()->master_volume()->setValue (_parameters.master_volume);
}


void
Session::load_session (QString const& file_name)
{
	try {
		QDomDocument document;

		// Open file:
		QFile file (file_name);
		if (!file.open (QFile::ReadOnly))
			throw Exception ((QString ("Could not open session file: ") + file.errorString()).toUtf8().data());
		else if (!document.setContent (&file, true))
			throw Exception ("Failed to parse session file.");

		// Process file:
		if (document.documentElement().tagName() == "haruhi-session")
			load_state (document.documentElement());
		else
			throw Exception ("Failed to process session file: invalid XML root element.");

		_file_name = file_name;

		// Add session to recent sessions list:
		SessionLoaderSettings* settings = Haruhi::haruhi()->session_loader_settings();
		settings->recent_sessions().push_back (SessionLoaderSettings::RecentSession (_name, file_name, Time::now()));
		settings->save();
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Error while loading session", Qt::escape (e.what()));
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
		if (!file.open (QFile::WriteOnly))
			throw Exception ((QString ("Could not save session file: ") + file.errorString()).toUtf8().data());
		QTextStream ts (&file);
		ts << document.toString();
		file.flush();
		file.close();
		::rename ((file_name + "~").toUtf8(), file_name.toUtf8());

		// Add session to recent sessions list:
		SessionLoaderSettings* settings = Haruhi::haruhi()->session_loader_settings();
		settings->recent_sessions().push_back (SessionLoaderSettings::RecentSession (_name, file_name, Time::now()));
		settings->save();
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Error while loading session", Qt::escape (e.what()));
	}
}


Frequency
Session::master_tune() const
{
	return 440_Hz * std::pow (2.0f, (1.0f / 12.0f) * (_parameters.tuning / 100.0f + _parameters.transpose));
}


void
Session::update_level_meters()
{
	if (_audio_backend)
	{
		AudioBackend::LevelsMap levels_map;
		std::vector<AudioPort*> ports;

		// Hold lock until we finish operations on Ports (sorting by name):
		graph()->synchronize ([&] {
			_audio_backend->peak_levels (levels_map);

			// Sort ports by name:
			for (auto& p: levels_map)
				ports.push_back (p.first);
			std::sort (ports.begin(), ports.end(), AudioPort::compare_by_name);
		});

		// Update level meter widget:
		for (unsigned int i = 0; i < std::min (ports.size(), static_cast<std::vector<AudioPort*>::size_type> (2u)); ++i)
			meter_panel()->level_meters_group()->meter (i)->set (levels_map[ports[i]]);
	}
}


void
Session::set_master_volume (Sample value, bool update_widget)
{
	if (!_audio_backend)
		return;
	_audio_backend->set_master_volume (FastPow::pow (value, M_E));
	if (update_widget)
		QApplication::postEvent (this, new UpdateMasterVolume (value));
}


void
Session::load_state (QDomElement const& element)
{
	// Stop processing, if any runs:
	delete _engine;
	_engine = 0;

	set_name (element.attribute ("name", "").toUtf8().data());

	QDomElement parameters_element;
	QDomElement audio_backend_element;
	QDomElement event_backend_element;
	QDomElement program_element;

	for (QDomElement& e: element)
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

	if (!audio_backend_element.isNull() && !event_backend_element.isNull() && !program_element.isNull())
	{
		// Components must be restored in given order (backends must get their
		// IDs before program and connections between units are restored):

		try {
			dynamic_cast<SaveableState&> (*_audio_backend).load_state (audio_backend_element);
		}
		catch (std::bad_cast const&)
		{ }

		try {
			dynamic_cast<SaveableState&> (*_event_backend).load_state (event_backend_element);
		}
		catch (std::bad_cast const&)
		{ }

		_engine = new Engine (this);
		_program->load_state (program_element);

		// Restore connections, parameters, etc. before starting engine:
		if (!parameters_element.isNull())
		{
			parameters().load_state (parameters_element);
			_session_global->load_params();
			_haruhi_global->load_params();
			apply_parameters();
		}

		// Restart processing:
		_engine->start();
	}
	else
		QMessageBox::warning (this, "Error while loading session", "Could not load session due to missing information in session file.");
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
	try {
		dynamic_cast<SaveableState&> (*_audio_backend).save_state (audio_backend);
		audio_backend.setAttribute ("id", _audio_backend->id());
	}
	catch (std::bad_cast const&)
	{ }

	// Save event-backend:
	QDomElement event_backend = element.ownerDocument().createElement ("event-backend");
	try {
		dynamic_cast<SaveableState&> (*_event_backend).save_state (event_backend);
		event_backend.setAttribute ("id", _event_backend->id());
	}
	catch (std::bad_cast const&)
	{ }

	// Save programs:
	QDomElement program = element.ownerDocument().createElement ("program");
	_program->save_state (program);

	element.appendChild (parameters);
	element.appendChild (audio_backend);
	element.appendChild (event_backend);
	element.appendChild (program);
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
	file_dialog->setFileMode (QFileDialog::AnyFile);
	file_dialog->setAcceptMode (QFileDialog::AcceptSave);
	if (file_dialog->exec() == QFileDialog::Accepted)
	{
		_file_name = file_dialog->selectedFiles().front();
		if (!QString (_file_name).endsWith (".haruhi-session", Qt::CaseInsensitive))
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
	Mutex::Lock lock (*graph());
	graph()->set_tempo (1_Hz * new_tempo);
}


void
Session::panic_button_clicked()
{
	Mutex::Lock lock (*graph());
	graph()->panic();
}


void
Session::show_prev_plugin()
{
	_program->show_prev_plugin();
}


void
Session::show_next_plugin()
{
	_program->show_next_plugin();
}


void
Session::create_main_menu()
{
	_main_menu = new QMenu (this);

	_main_menu->addAction (Resources::Icons16::show_program(), "Program", this, SLOT (show_program()), Qt::Key_F1);
	_main_menu->addAction (Resources::Icons16::show_backends(), "Session settings", this, SLOT (show_session_settings()), Qt::Key_F2);
	_main_menu->addAction (Resources::Icons16::haruhi(), "Haruhi settings", this, SLOT (show_haruhi_settings()), Qt::Key_F3);
	_main_menu->addSeparator();
	_main_menu->addAction (Resources::Icons16::session_manager(), "Session &manager…", this, SLOT (session_loader()), Qt::CTRL + Qt::Key_M);
	_main_menu->addAction (Resources::Icons16::save(), "&Save", this, SLOT (save_session()), Qt::CTRL + Qt::Key_S);
	_main_menu->addAction (Resources::Icons16::save_as(), "Sa&ve as…", this, SLOT (save_session_as()), Qt::CTRL + Qt::SHIFT + Qt::Key_S);
	_main_menu->addSeparator();
	_main_menu->addAction (Resources::Icons16::disconnect(), "&Reconnect to JACK", this, SLOT (reconnect_to_jack()), Qt::CTRL + Qt::Key_J);
	_main_menu->addSeparator();
	_main_menu->addAction (Resources::Icons16::exit(), "&Quit", Haruhi::haruhi()->application(), SLOT (quit()), Qt::CTRL + Qt::Key_Q);
}


void
Session::stop_audio_backend()
{
	if (_audio_backend)
	{
		try {
			dynamic_cast<QWidget&> (*_audio_backend).hide();
		}
		catch (std::bad_cast const&)
		{ }
		_graph->unregister_audio_backend();
		delete _audio_backend;
	}
	_audio_backend = 0;
}


void
Session::stop_event_backend()
{
	if (_event_backend)
	{
		try {
			dynamic_cast<QWidget&> (*_event_backend).hide();
		}
		catch (std::bad_cast const&)
		{ }
		if (_devices_manager)
			_devices_manager->set_event_backend (0);
		_graph->unregister_event_backend();
		delete _event_backend;
	}
	_event_backend = 0;
}


void
Session::start_audio_backend()
{
	try {
		AudioBackendImpl::Backend* audio_backend = new AudioBackendImpl::Backend ("Haruhi", _audio_tab);
		_audio_tab->layout()->addWidget (audio_backend);
		_audio_backend = audio_backend;
		audio_backend->on_state_change.connect (this, &Session::audio_backend_state_change);
		audio_backend->show();
		_graph->register_audio_backend (_audio_backend);
		// Initially set master volume:
		master_volume_changed (meter_panel()->master_volume()->value());
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Could not create audio backend", "Could not start audio backend: " + Qt::escape (e.what()));
	}
}


void
Session::start_event_backend()
{
	try {
		EventBackendImpl::Backend* event_backend = new EventBackendImpl::Backend ("Haruhi", _event_tab);
		_event_tab->layout()->addWidget (event_backend);
		_event_backend = event_backend;
		event_backend->show();
		_graph->register_event_backend (_event_backend);
		// Reload DevicesManager list when EventBackend creates new template:
		if (!_devices_manager)
			throw Exception ("DevicesManager must be created before EventBackend");
		// Connect EventBackend and DevicesManager:
		event_backend->device_saved_as_template.connect (_devices_manager->settings(), &DevicesManager::Settings::add_device);
		event_backend->on_event.connect (_devices_manager, &DevicesManager::Panel::on_event);
		_devices_manager->set_event_backend (event_backend);
		_devices_manager->settings()->model().on_change.connect (event_backend, &EventBackendImpl::Backend::devices_manager_updated);
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Could not create event backend", "Could not start event backend: " + Qt::escape (e.what()));
	}
}


void
Session::audio_backend_state_change (bool)
{
	update_window_title();
}


void
Session::master_volume_changed (int value)
{
	_parameters.master_volume = value;
	set_master_volume (value / static_cast<float> (Session::MeterPanel::MaxVolume), false);
}


void
Session::update_window_title()
{
	QString s ("Haruhi");
	if (!_audio_backend || !_audio_backend->connected())
		s = "Haruhi (offline)";
	setWindowTitle (s);
}


void
Session::closeEvent (QCloseEvent* e)
{
	e->accept();
	Haruhi::haruhi()->ok_to_quit();
}


void
Session::customEvent (QEvent* e)
{
	UpdateMasterVolume* ue = dynamic_cast<UpdateMasterVolume*> (e);
	if (!ue)
		return;
	e->accept();
	if (_audio_backend)
		meter_panel()->master_volume()->setValue (renormalize (ue->value, 0.0f, 1.0f, Session::MeterPanel::MinVolume, Session::MeterPanel::MaxVolume));
}


QWidget*
Session::create_container (QWidget* parent)
{
	// Configure layouts for audio and event tabs:
	QWidget* w = new QWidget (parent);
	w->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QHBoxLayout* layout = new QHBoxLayout (w);
	layout->setMargin (0);
	layout->setSpacing (0);
	return w;
}

} // namespace Haruhi

