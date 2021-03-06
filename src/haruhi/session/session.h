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

#ifndef HARUHI__SESSION__SESSION_H__INCLUDED
#define HARUHI__SESSION__SESSION_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QWidget>
#include <QTabWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>
#include <QStackedWidget>
#include <QMenu>
#include <QDomNode>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/graph.h>
#include <haruhi/components/audio_backend/backend.h>
#include <haruhi/components/event_backend/backend.h>
#include <haruhi/components/devices_manager/panel.h>
#include <haruhi/plugin/plugin_loader.h>
#include <haruhi/session/engine.h>
#include <haruhi/session/program.h>
#include <haruhi/utility/thread.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/signal.h>
#include <haruhi/utility/work_performer.h>
#include <haruhi/utility/noncopyable.h>
#include <haruhi/widgets/level_meter.h>
#include <haruhi/widgets/dial_control.h>


namespace Haruhi {

class Session;

namespace SessionPrivate {

	class SettingsDialog: public QDialog
	{
		Q_OBJECT

	  public:
		SettingsDialog (QWidget* parent, Session* session);

		virtual ~SettingsDialog() = default;

		QString
		name() const;

		void
		apply();

	  private slots:
		void
		state_changed();

		void
		validate_and_accept();

	  private:
		Session*			_session;
		Unique<QLineEdit>	_name;
		Unique<QPushButton>	_accept_button;
		Unique<QPushButton>	_reject_button;
	};

	class SessionGlobal: public QWidget
	{
		Q_OBJECT

		friend class Session;

	  public:
		SessionGlobal (Session*, QWidget* parent);

		void
		load_params();

	  private slots:
		void
		update_params();

		void
		update_widgets();

	  private:
		Session*			_session;
		bool				_loading_params;

		Unique<QSpinBox>	_tuning;
		Unique<QLabel>		_tuning_hz;
		Unique<QSpinBox>	_transpose;
	};

	class HaruhiGlobal: public QWidget
	{
		Q_OBJECT

		friend class Session;

	  public:
		HaruhiGlobal (Session*, QWidget* parent);

		void
		load_params();

	  private slots:
		void
		update_params();

		void
		update_widgets();

	  private:
		Session*			_session;
		bool				_loading_params;

		Unique<QSpinBox>	_engine_thread_priority;
		Unique<QSpinBox>	_level_meter_fps;
	};

} // namespace SessionPrivate


class Session:
	public QWidget,
	public Signal::Receiver,
	public SaveableState,
	public Noncopyable
{
	Q_OBJECT

	friend class SessionPrivate::SettingsDialog;

  public:
	/**
	 * Session parameters. Will be saved to session file.
	 */
	class Parameters: public SaveableState
	{
	  public:
		Parameters();

		void
		load_state (QDomElement const& element) override;

		void
		save_state (QDomElement&) const override;

	  private:
		void
		limit_values();

	  public:
		int		tuning			= 0; // -50…50 cents
		int		transpose		= 0;
		float	tempo			= 120.0;
		int		master_volume	= 0;
	};

	class MeterPanel: public QFrame
	{
	  public:
		enum {
			MinVolume	= 0,
			ZeroVolume	= 1000,
			MaxVolume	= 1200,
		};

	  public:
		MeterPanel (Session*, QWidget* parent);

		LevelMetersGroup*
		level_meters_group() const;

		DialControl*
		master_volume() const;

		void
		set_fps (int fps);

	  private:
		Session*					_session;
		Unique<LevelMetersGroup>	_level_meters_group;
		Unique<DialControl>			_master_volume;
	};

	class UpdateMasterVolume: public QEvent
	{
	  public:
		UpdateMasterVolume (Sample value);

		Sample value;
	};

  public:
	Session (QWidget* parent);

	virtual ~Session();

	Engine*
	engine() const;

	QString const&
	name() const;

	void
	set_name (QString const& name);


	QString const&
	file_name() const;

	void
	set_file_name (QString const& file_name);

	Graph*
	graph() const;

	PluginLoader*
	plugin_loader() const;

	MeterPanel*
	meter_panel() const;

	Parameters&
	parameters();

	DevicesManager::Panel*
	devices_manager();

	/**
	 * Applies _parameters to system.
	 */
	void
	apply_parameters();

	void
	load_session (QString const& file_name);

	void
	save_session (QString const& file_name);

	Frequency
	master_tune() const;

	void
	update_level_meters();

	void
	set_master_volume (Sample value, bool update_widget = true);

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement&) const override;

	void
	load_state (QDomElement const&) override;

  private slots:
	void
	session_loader();

	void
	save_session();

	void
	save_session_as();

	void
	rename_session();

	void
	tempo_value_changed (double);

	void
	panic_button_clicked();

	void
	show_prev_plugin();

	void
	show_next_plugin();

	void
	create_main_menu();

	void
	stop_audio_backend();

	void
	stop_event_backend();

	void
	start_audio_backend();

	void
	start_event_backend();

	void
	audio_backend_state_change (bool);

	void
	show_program();

	void
	show_session_settings();

	void
	show_haruhi_settings();

	void
	reconnect_to_jack();

	void
	master_volume_changed (int);

	void
	update_window_title();

  protected:
	void
	closeEvent (QCloseEvent*) override;

	void
	customEvent (QEvent*) override;

  private:
	Unique<QWidget>
	create_container (QWidget* parent);

  private:
	QString									_name;
	QString									_file_name;
	Parameters								_parameters;

	Unique<MeterPanel>						_meter_panel;
	Unique<QStackedWidget>					_stack;
	Unique<QLabel>							_session_name;
	Unique<QPushButton>						_panic_button;
	Unique<QPushButton>						_main_menu_button;
	Unique<QDoubleSpinBox>					_tempo_spinbox;
	Unique<QMenu>							_main_menu;

	// In this order:
	Unique<Graph>							_graph;
	Unique<Engine>							_engine;
	Unique<PluginLoader>					_plugin_loader;
	Unique<Program>							_program;

	Unique<QTabWidget>						_session_settings;
	Unique<QTabWidget>						_haruhi_settings;
	Unique<SessionPrivate::SessionGlobal>	_session_global;
	Unique<SessionPrivate::HaruhiGlobal>	_haruhi_global;
	Unique<QWidget>							_audio_widget;
	Unique<QWidget>							_event_widget;

	// Links to main session components:
	Unique<AudioBackend>					_audio_backend;
	Unique<EventBackend>					_event_backend;
	Unique<DevicesManager::Panel>			_devices_manager;
};


inline LevelMetersGroup*
Session::MeterPanel::level_meters_group() const
{
	return _level_meters_group.get();
}


inline DialControl*
Session::MeterPanel::master_volume() const
{
	return _master_volume.get();
}


inline
Session::UpdateMasterVolume::UpdateMasterVolume (Sample value):
	QEvent (QEvent::User),
	value (value)
{ }


inline Engine*
Session::engine() const
{
	return _engine.get();
}


inline QString const&
Session::name() const
{
	return _name;
}


inline void
Session::set_name (QString const& name)
{
	_name = name;
	_session_name->setText (name);
}


inline QString const&
Session::file_name() const
{
	return _file_name;
}


inline void
Session::set_file_name (QString const& file_name)
{
	_file_name = file_name;
}


inline Graph*
Session::graph() const
{
	return _graph.get();
}


inline PluginLoader*
Session::plugin_loader() const
{
	return _plugin_loader.get();
}


inline Session::MeterPanel*
Session::meter_panel() const
{
	return _meter_panel.get();
}


inline Session::Parameters&
Session::parameters()
{
	return _parameters;
}


inline DevicesManager::Panel*
Session::devices_manager()
{
	return _devices_manager.get();
}


inline void
Session::show_program()
{
	_stack->setCurrentWidget (_program.get());
}


inline void
Session::show_session_settings()
{
	_stack->setCurrentWidget (_session_settings.get());
}


inline void
Session::show_haruhi_settings()
{
	_stack->setCurrentWidget (_haruhi_settings.get());
}


inline void
Session::reconnect_to_jack()
{
	_audio_backend->connect();
}

} // namespace Haruhi

#endif

