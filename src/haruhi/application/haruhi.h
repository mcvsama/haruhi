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

#ifndef HARUHI__APPLICATION__HARUHI_H__INCLUDED
#define HARUHI__APPLICATION__HARUHI_H__INCLUDED

// Standard:
#include <cstddef>

// System:
#include <unistd.h>

// Lib:
#include <QtCore/QObject>
#include <QtGui/QApplication>
#include <QtGui/QStackedWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QFrame>
#include <QtGui/QLabel>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/components/devices_manager/settings.h>
#include <haruhi/settings/settings.h>
#include <haruhi/settings/has_presets_settings.h>
#include <haruhi/settings/session_loader_settings.h>
#include <haruhi/utility/noncopyable.h>
#include <haruhi/utility/backtrace.h>


namespace Haruhi {

class UnitManager;
class Session;
class PeriodicUpdater;

/**
 * Haruhi/system - stores shared data.
 */
class Haruhi:
	public QObject,
	private Noncopyable
{
	Q_OBJECT

  public:
	Haruhi (int argc, char** argv, char** envp);

	~Haruhi();

	void
	run_ui();

	QApplication*
	application() const { return _app; }

	/**
	 * \returns	reference to main system.
	 */
	static Haruhi*
	haruhi() { return _haruhi; }

	/**
	 * \returns	detected processor cores.
	 */
	static int
	detected_cores();

	Session*
	session() const { return _session; }

	Settings*
	settings() const { return _settings; }

	DevicesManager::Settings*
	devices_manager_settings() const { return _devices_manager_settings; }

	HasPresetsSettings*
	has_presets_settings() const { return _has_presets_settings; }

	SessionLoaderSettings*
	session_loader_settings() const { return _session_loader_settings; }

  public slots:
	void
	session_loader();

	void
	ok_to_quit();

	void
	quit_if_ok();

  private:
	// Haruhi pointer:
	static Haruhi*				_haruhi;
	static int					_detected_cores;

	// Links to main application components:
	QApplication*				_app;
	PeriodicUpdater*			_periodic_updater;
	Session*					_session;
	Settings*					_settings;
	DevicesManager::Settings*	_devices_manager_settings;
	HasPresetsSettings*			_has_presets_settings;
	SessionLoaderSettings*		_session_loader_settings;
	bool						_ok_to_quit;

	// Other:
	int		_argc;
	char**	_argv;
	char**	_envp;
};

} // namespace Haruhi

#endif

