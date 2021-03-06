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
#include <QFile>
#include <QObjectList>
#include <QTextStream>
#include <QApplication>
#include <QShortcut>
#include <QCursor>
#include <QMessageBox>
#include <QWhatsThis>
#include <QSlider>
#include <QFileDialog>
#include <QCloseEvent>
#include <QGridLayout>
#include <QLayout>
#include <QTextDocument>

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

using namespace ScreenLiterals;
namespace Private = SessionPrivate;


Private::SettingsDialog::SettingsDialog (QWidget* parent, Session* session):
	QDialog (parent),
	_session (session)
{
	setWindowTitle ("Session properties");
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setMinimumWidth (300);

	auto name_label = new QLabel ("Session name:", this);

	_name = std::make_unique<QLineEdit> (_session->_name, this);
	_name->selectAll();
	_name->setFocus();
	QObject::connect (_name.get(), SIGNAL (textChanged (const QString&)), this, SLOT (state_changed()));

	_accept_button = std::make_unique<QPushButton> ("&Ok", this);
	_accept_button->setIconSize (Resources::Icons16::haruhi().size());
	_accept_button->setDefault (true);
	QObject::connect (_accept_button.get(), SIGNAL (clicked()), this, SLOT (validate_and_accept()));

	_reject_button = std::make_unique<QPushButton> ("&Cancel", this);
	_reject_button->setIconSize (Resources::Icons16::haruhi().size());
	QObject::connect (_reject_button.get(), SIGNAL (clicked()), this, SLOT (reject()));

	// Layouts:

	auto name_layout = new QHBoxLayout();
	name_layout->setSpacing (Config::spacing());
	name_layout->addWidget (name_label);
	name_layout->addWidget (_name.get());

	auto buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::spacing());
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (_accept_button.get());
	buttons_layout->addWidget (_reject_button.get());

	auto layout = new QVBoxLayout (this);
	layout->setMargin (Config::dialog_margin());
	layout->setSpacing (Config::spacing());
	layout->addLayout (name_layout);
	layout->addLayout (buttons_layout);

	state_changed();

	adjustSize();
	setFixedHeight (height());
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

	_tuning_hz = std::make_unique<QLabel> (this);

	_tuning = std::make_unique<QSpinBox> (this);
	_tuning->setRange (-50, +50);
	_tuning->setSuffix (" cents");
	_tuning->setValue (0);
	QObject::connect (_tuning.get(), SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	_transpose = std::make_unique<QSpinBox> (this);
	_transpose->setRange (-60, 60);
	_transpose->setSuffix (" semitones");
	_transpose->setValue (0);
	QObject::connect (_transpose.get(), SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	auto group_layout = new QGridLayout (this);
	group_layout->addWidget (new QLabel ("Master tuning:", this), 0, 0);
	group_layout->addWidget (_tuning.get(), 0, 1);
	group_layout->addWidget (_tuning_hz.get(), 0, 2);
	group_layout->addWidget (new QLabel ("Transpose:", this), 1, 0);
	group_layout->addWidget (_transpose.get(), 1, 1);
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

	_engine_thread_priority = std::make_unique<QSpinBox> (this);
	_engine_thread_priority->setRange (1, 99);
	_engine_thread_priority->setValue (50);
	_engine_thread_priority->setToolTip ("Higher values mean higher priority");
	QObject::connect (_engine_thread_priority.get(), SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	_level_meter_fps = std::make_unique<QSpinBox> (this);
	_level_meter_fps->setRange (10, 50);
	_level_meter_fps->setValue (30);
	QObject::connect (_level_meter_fps.get(), SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	auto group_layout = new QGridLayout (this);
	group_layout->addWidget (new QLabel ("Engine thread priority:", this), 0, 0);
	group_layout->addWidget (_engine_thread_priority.get(), 0, 1);
	group_layout->addWidget (new QLabel ("Level Meter FPS:", this), 1, 0);
	group_layout->addWidget (_level_meter_fps.get(), 1, 1);
	group_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 2);
	group_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 2, 0);

	update_widgets();
}


void
Private::HaruhiGlobal::load_params()
{
	auto haruhi_settings = Haruhi::haruhi()->haruhi_settings();

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

	auto haruhi_settings = Haruhi::haruhi()->haruhi_settings();

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


Session::Parameters::Parameters()
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
	auto par_tuning = element.ownerDocument().createElement ("tuning");
	par_tuning.appendChild (element.ownerDocument().createTextNode (QString::number (tuning)));

	auto par_transpose = element.ownerDocument().createElement ("transpose");
	par_transpose.appendChild (element.ownerDocument().createTextNode (QString::number (transpose)));

	auto par_tempo = element.ownerDocument().createElement ("tempo");
	par_tempo.appendChild (element.ownerDocument().createTextNode (QString::number (tempo)));

	auto par_master_volume = element.ownerDocument().createElement ("master-volume");
	par_master_volume.appendChild (element.ownerDocument().createTextNode (QString::number (master_volume)));

	element.appendChild (par_tuning);
	element.appendChild (par_transpose);
	element.appendChild (par_tempo);
	element.appendChild (par_master_volume);
}


void
Session::Parameters::limit_values()
{
	clamp (tuning, -50, 50);
	clamp (transpose, -60, 60);
	clamp (tempo, 20.0f, 400.0f);
	clamp (master_volume, static_cast<int> (MeterPanel::MinVolume), static_cast<int> (MeterPanel::MaxVolume));
}


Session::MeterPanel::MeterPanel (Session* session, QWidget* parent):
	QFrame (parent),
	_session (session)
{
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
	setFrameStyle (QFrame::StyledPanel | QFrame::Raised);

	_level_meters_group = std::make_unique<LevelMetersGroup> (this);
	_master_volume = std::make_unique<DialControl> (this, Range<int> { MinVolume, MaxVolume }, ZeroVolume * std::pow (attenuate_db (-3.0f), 1.0f / M_E));
	_master_volume->setToolTip ("Master Volume");
	QObject::connect (_master_volume.get(), SIGNAL (valueChanged (int)), _session, SLOT (master_volume_changed (int)));

	auto layout = new QVBoxLayout (this);
	layout->setMargin (Config::margin());
	layout->setSpacing (Config::spacing());
	layout->addWidget (_level_meters_group.get());
	layout->addWidget (_master_volume.get());
}


Session::Session (QWidget* parent):
	QWidget (parent),
	_graph (std::make_unique<Graph>()),
	_plugin_loader (std::make_unique<PluginLoader>())
{
	_name = "";

	setWindowIcon (QPixmap ("share/images/haruhi.png"));
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	create_main_menu();

	auto header = new QFrame (this);
	header->setAutoFillBackground (true);
	QPalette p = header->palette();
	p.setColor (QPalette::Window, QColor (0xda, 0xe1, 0xe9));
	header->setPalette (p);
	header->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	header->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	auto inner_header = new QWidget (header);
	inner_header->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	_session_name = std::make_unique<ClickableLabel> (_name, inner_header);
	QFont f (QApplication::font());
	f.setPixelSize (1.6_em);
	f.setWeight (QFont::Normal);
	_session_name->setFont (f);
	_session_name->setCursor (QCursor (Qt::PointingHandCursor));
	_session_name->setTextFormat (Qt::PlainText);
	_session_name->setToolTip ("Click to rename session");
	QObject::connect (_session_name.get(), SIGNAL (clicked()), this, SLOT (rename_session()));

	auto tempo_note = new QLabel (QString::fromUtf8 ("♩ = "), inner_header);
	f.setPixelSize (1.6_em);
	f.setWeight (QFont::Normal);
	tempo_note->setFont (f);
	tempo_note->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	_tempo_spinbox = std::make_unique<QDoubleSpinBox> (inner_header);
	_tempo_spinbox->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_tempo_spinbox->setRange (20.0, 400.0);
	_tempo_spinbox->setDecimals (2);
	_tempo_spinbox->setWrapping (true);
	_tempo_spinbox->setValue (120.0);
	_tempo_spinbox->setToolTip ("Master tempo");
	QObject::connect (_tempo_spinbox.get(), SIGNAL (valueChanged (double)), this, SLOT (tempo_value_changed (double)));

	_panic_button = std::make_unique<QPushButton> (Resources::Icons16::panic(), "Panic!", inner_header);
	_panic_button->setIconSize (Resources::Icons16::haruhi().size());
	_panic_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_panic_button->setToolTip ("Stops all sound processing (F10)");
	QObject::connect (_panic_button.get(), SIGNAL (clicked()), this, SLOT (panic_button_clicked()));
	new QShortcut (Qt::Key_F10, this, SLOT (panic_button_clicked()));
	new QShortcut (Qt::ControlModifier + Qt::Key_PageUp, this, SLOT (show_prev_plugin()));
	new QShortcut (Qt::ControlModifier + Qt::Key_PageDown, this, SLOT (show_next_plugin()));

	_main_menu_button = std::make_unique<QPushButton> ("Menu ", inner_header);
	_main_menu_button->setIconSize (Resources::Icons16::haruhi().size());
	_main_menu_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_main_menu_button->setMenu (_main_menu.get());
	_main_menu_button->setIcon (Resources::Icons16::menu());

	_meter_panel = std::make_unique<MeterPanel> (this, this);
	_stack = std::make_unique<QStackedWidget> (this);

	_session_settings = std::make_unique<QTabWidget> (this);
	_session_settings->setTabPosition (QTabWidget::South);
	_session_settings->setIconSize (Resources::Icons16::haruhi().size() * 1.25);

	_haruhi_settings = std::make_unique<QTabWidget> (this);
	_haruhi_settings->setTabPosition (QTabWidget::South);
	_haruhi_settings->setIconSize (Resources::Icons16::haruhi().size() * 1.25);

	_session_global = std::make_unique<Private::SessionGlobal> (this, _session_settings.get());
	_haruhi_global = std::make_unique<Private::HaruhiGlobal> (this, _session_settings.get());
	_audio_widget = create_container (this);
	_event_widget = create_container (this);

	// Contains audio & event widgets:
	auto ports_tab = new QWidget (this);
	auto ports_tab_layout = new QHBoxLayout (ports_tab);
	ports_tab_layout->setMargin (Config::margin());
	ports_tab_layout->setSpacing (Config::spacing());
	ports_tab_layout->addWidget (_audio_widget.get());
	ports_tab_layout->addWidget (_event_widget.get());

	_devices_manager = std::make_unique<DevicesManager::Panel> (this, Haruhi::haruhi()->devices_manager_settings());

	// Layouts:

	auto header_layout = new QHBoxLayout (header);
	header_layout->setMargin (Config::margin());
	header_layout->addWidget (inner_header);

	auto inner_header_layout = new QHBoxLayout (inner_header);
	inner_header_layout->setMargin (0);
	inner_header_layout->setSpacing (Config::spacing());
	inner_header_layout->addWidget (_session_name.get());
	inner_header_layout->addItem (new QSpacerItem (0, 0));
	inner_header_layout->addWidget (tempo_note);
	inner_header_layout->addWidget (_tempo_spinbox.get());
	inner_header_layout->addItem (new QSpacerItem (2 * Config::spacing(), 0, QSizePolicy::Maximum, QSizePolicy::Minimum));
	inner_header_layout->addWidget (_panic_button.get());
	inner_header_layout->addWidget (_main_menu_button.get());

	auto bottom_layout = new QHBoxLayout();
	bottom_layout->setSpacing (Config::spacing() + 1);
	bottom_layout->addWidget (_meter_panel.get());
	bottom_layout->addWidget (_stack.get());

	auto layout = new QVBoxLayout (this);
	layout->setMargin (Config::window_margin());
	layout->setSpacing (Config::spacing());
	layout->addWidget (header);
	layout->addLayout (bottom_layout);

	// Add tabs:
	_session_settings->addTab (_session_global.get(), Resources::Icons16::configure(), "Session settings");
	_session_settings->addTab (ports_tab, Resources::Icons16::audio(), "Audio && event ports");

	_haruhi_settings->addTab (_haruhi_global.get(), Resources::Icons16::configure(), "Haruhi settings");
	_haruhi_settings->addTab (_devices_manager.get(), Resources::Icons16::keyboard(), "Device templates");

	// Start engine and backends before program is loaded:
	_engine = std::make_unique<Engine> (this);
	start_event_backend();
	start_audio_backend();
	_engine->start();

	_program = std::make_unique<Program> (this, _stack.get());

	_stack->addWidget (_program.get());
	_stack->addWidget (_session_settings.get());
	_stack->addWidget (_haruhi_settings.get());

	_stack->setCurrentWidget (_program.get());

	update_window_title();
}


Session::~Session()
{
	// In this order:
	_program.reset();
	_plugin_loader.reset();

	_engine.reset();
	stop_audio_backend();
	stop_event_backend();
	_graph.reset();
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
		QMessageBox::warning (this, "Error while loading session", QString (e.what()).toHtmlEscaped());
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
		QMessageBox::warning (this, "Error while loading session", QString (e.what()).toHtmlEscaped());
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
	_engine.reset();

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

		if (auto sst = dynamic_cast<SaveableState*> (_audio_backend.get()))
			sst->load_state (audio_backend_element);

		if (auto sst = dynamic_cast<SaveableState*> (_event_backend.get()))
			sst->load_state (event_backend_element);

		_engine = std::make_unique<Engine> (this);
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
	if (auto sst = dynamic_cast<SaveableState*> (_audio_backend.get()))
	{
		sst->save_state (audio_backend);
		audio_backend.setAttribute ("id", _audio_backend->id());
	}

	// Save event-backend:
	QDomElement event_backend = element.ownerDocument().createElement ("event-backend");
	if (auto sst = dynamic_cast<SaveableState*> (_event_backend.get()))
	{
		sst->save_state (event_backend);
		event_backend.setAttribute ("id", _event_backend->id());
	}

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
	auto file_dialog = new QFileDialog (this, "Save session", ".", QString());
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
	auto dialog = new Private::SettingsDialog (this, this);
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
	_main_menu = std::make_unique<QMenu> (this);

	_main_menu->addAction ("Program", this, SLOT (show_program()), Qt::Key_F1);
	_main_menu->addAction ("Session settings", this, SLOT (show_session_settings()), Qt::Key_F2);
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
		if (auto w = dynamic_cast<QWidget*> (_audio_backend.get()))
			w->hide();

		_graph->unregister_audio_backend();
		_audio_backend.reset();
	}
}


void
Session::stop_event_backend()
{
	if (_event_backend)
	{
		if (auto w = dynamic_cast<QWidget*> (_event_backend.get()))
			w->hide();

		if (_devices_manager)
			_devices_manager->set_event_backend (nullptr);

		_graph->unregister_event_backend();
		_event_backend.reset();
	}
}


void
Session::start_audio_backend()
{
	try {
		auto audio_backend = std::make_unique<AudioBackendImpl::Backend> ("Haruhi", _audio_widget.get());
		_audio_widget->layout()->addWidget (audio_backend.get());
		audio_backend->on_state_change.connect (this, &Session::audio_backend_state_change);
		audio_backend->show();
		_audio_backend = std::move (audio_backend);
		_graph->register_audio_backend (_audio_backend.get());
		// Initially set master volume:
		master_volume_changed (meter_panel()->master_volume()->value());
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Could not create audio backend", "Could not start audio backend: " + QString (e.what()).toHtmlEscaped());
	}
}


void
Session::start_event_backend()
{
	try {
		auto event_backend = std::make_unique<EventBackendImpl::Backend> ("Haruhi", _event_widget.get());
		_event_widget->layout()->addWidget (event_backend.get());
		event_backend->show();
		_graph->register_event_backend (event_backend.get());
		// Reload DevicesManager list when EventBackend creates new template:
		if (!_devices_manager)
			throw Exception ("DevicesManager must be created before EventBackend");
		// Connect EventBackend and DevicesManager:
		event_backend->device_saved_as_template.connect (_devices_manager->settings(), &DevicesManager::Settings::add_device);
		event_backend->on_event.connect (_devices_manager.get(), &DevicesManager::Panel::on_event);
		_devices_manager->set_event_backend (event_backend.get());
		_devices_manager->settings()->model().on_change.connect (event_backend.get(), &EventBackendImpl::Backend::devices_manager_updated);
		_event_backend = std::move (event_backend);
	}
	catch (Exception const& e)
	{
		QMessageBox::warning (this, "Could not create event backend", "Could not start event backend: " + QString (e.what()).toHtmlEscaped());
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
	if (auto ue = dynamic_cast<UpdateMasterVolume*> (e))
	{
		e->accept();
		if (_audio_backend)
			meter_panel()->master_volume()->setValue (renormalize (ue->value, 0.0f, 1.0f, Session::MeterPanel::MinVolume, Session::MeterPanel::MaxVolume));
	}
}


Unique<QWidget>
Session::create_container (QWidget* parent)
{
	// Configure layouts for audio and event tabs:
	auto w = std::make_unique<QFrame> (parent);
	w->setFrameShadow (QFrame::Raised);
	w->setFrameShape (QFrame::StyledPanel);
	w->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	auto layout = new QVBoxLayout (w.get());
	layout->setMargin (Config::margin());
	layout->setSpacing (Config::spacing());

	return w;
}

} // namespace Haruhi

