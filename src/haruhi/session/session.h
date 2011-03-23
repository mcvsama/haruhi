/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
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
#include <QtGui/QWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QSpinBox>
#include <QtGui/QPushButton>
#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QStackedWidget>
#include <QtGui/QMenu>
#include <QtXml/QDomNode>

// Haruhi:
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

		virtual ~SettingsDialog() { }

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
		Session*		_session;
		QLineEdit*		_name;
		QPushButton*	_accept_button;
		QPushButton*	_reject_button;
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
		Session*	_session;
		bool		_loading_params;

		QSpinBox*	_tuning;
		QLabel*		_tuning_hz;
		QSpinBox*	_transpose;
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
		Session*	_session;
		bool		_loading_params;

		QSpinBox*	_engine_thread_priority;
		QSpinBox*	_level_meter_fps;
	};

} // namespace SessionPrivate


class Session:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

  public:
	/**
	 * Session parameters. Will be saved to session file.
	 */
	class Parameters: public SaveableState
	{
	  public:
		Parameters();

		void
		load_state (QDomElement const& element);

		void
		save_state (QDomElement&) const;

	  private:
		void
		limit_values();

	  public:
		int		tuning; // -50…50 cents
		int		transpose;
		float	tempo;
		int		master_volume;
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
		level_meters_group() const { return _level_meters_group; }

		DialControl*
		master_volume() const { return _master_volume; }

		void
		set_fps (int fps);

	  private:
		Session*			_session;
		LevelMetersGroup*	_level_meters_group;
		DialControl*		_master_volume;
	};

	class UpdateMasterVolume: public QEvent
	{
	  public:
		UpdateMasterVolume (Sample value):
			QEvent (QEvent::User),
			value (value)
		{ }

		Sample value;
	};

  private:
	// Prevent copying:
	Session (Session const&);
	Session& operator= (Session const&);

	friend class SessionPrivate::SettingsDialog;

  public:
	Session (QWidget* parent);

	virtual ~Session();

	Engine*
	engine() const { return _engine; }

	QString const&
	name() const { return _name; }

	void
	set_name (QString const& name)
	{
		_name = name;
		_session_name->setText (name);
	}

	QString const&
	file_name() const { return _file_name; }

	void
	set_file_name (QString const& file_name) { _file_name = file_name; }

	Graph*
	graph() const { return _graph; }

	PluginLoader*
	plugin_loader() const { return _plugin_loader; }

	MeterPanel*
	meter_panel() const { return _meter_panel; }

	Parameters&
	parameters() { return _parameters; }

	DevicesManager::Panel*
	devices_manager() { return _devices_manager; }

	/**
	 * Applies _parameters to system.
	 */
	void
	apply_parameters();

	void
	load_session (QString const& file_name);

	void
	save_session (QString const& file_name);

	float
	master_tune() const;

	void
	update_level_meters();

	void
	set_master_volume (Sample value, bool update_widget = true);

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

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
	show_program() { _stack->setCurrentWidget (_program); }

	void
	show_session_settings() { _stack->setCurrentWidget (_session_settings); }

	void
	show_haruhi_settings() { _stack->setCurrentWidget (_haruhi_settings); }

	void
	reconnect_to_jack() { _audio_backend->connect(); }

	void
	master_volume_changed (int);

	void
	update_window_title();

  protected:
	void
	closeEvent (QCloseEvent*);

	void
	customEvent (QEvent*);

  private:
	QWidget*
	create_container (QWidget* parent);

  private:
	QString							_name;
	QString							_file_name;
	Parameters						_parameters;
	Graph*							_graph;

	MeterPanel*						_meter_panel;
	QStackedWidget*					_stack;
	QLabel*							_session_name;
	QPushButton*					_panic_button;
	QPushButton*					_main_menu_button;

	Program*						_program;
	QTabWidget*						_session_settings;
	QTabWidget*						_haruhi_settings;

	SessionPrivate::SessionGlobal*	_session_global;
	SessionPrivate::HaruhiGlobal*	_haruhi_global;
	QWidget*						_audio_tab;
	QWidget*						_event_tab;

	// Links to main session components:
	AudioBackend*					_audio_backend;
	EventBackend*					_event_backend;
	Engine*							_engine;
	PluginLoader*					_plugin_loader;
	DevicesManager::Panel*			_devices_manager;

	QDoubleSpinBox*					_tempo_spinbox;
	QMenu*							_main_menu;
};

} // namespace Haruhi

#endif

