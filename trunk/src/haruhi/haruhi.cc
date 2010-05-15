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
#include <fstream>
#include <sstream>
#include <string>

// Lib:
#include <QtGui/QApplication>
#include <QtGui/QStackedWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QLayout>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/session.h>
#include <haruhi/session_loader.h>
#include <haruhi/periodic_updater.h>

// Local:
#include "haruhi.h"


namespace Haruhi {

Haruhi* Haruhi::_haruhi = 0;
int Haruhi::_detected_cores = -1;


Haruhi::Haruhi (int argc, char** argv, char** envp):
	_session (0),
	_ok_to_quit (false),
	_argc (argc),
	_argv (argv),
	_envp (envp)
{
	if (_haruhi)
		throw Exception ("only one instance of Haruhi is allowed");

	_haruhi = this;

	Config::initialize ("haruhi");
	this->run_ui();
	Config::deinitialize();
}


Haruhi::~Haruhi()
{
	_haruhi = 0;
}


void
Haruhi::run_ui()
{
	_app = new QApplication (_argc, _argv);
	_periodic_updater = new PeriodicUpdater (30);
	QObject::connect (_app, SIGNAL (lastWindowClosed()), this, SLOT (quit_if_ok()));
	session_loader();
	if (_session)
		_app->exec();
	// Close current session:
	delete _session;
	delete _periodic_updater;
	delete _app;
}


int
Haruhi::detected_cores()
{
	if (_detected_cores != -1)
		return _detected_cores;

	_detected_cores = 0;
	std::ifstream cpuinfo ("/proc/cpuinfo");
	std::string line;
	while (cpuinfo.good())
	{
		std::getline (cpuinfo, line);
		std::istringstream s (line);
		std::string name, colon;
		s >> name >> colon;
		if (name == "processor" && colon == ":")
			++_detected_cores;
	}
	return _detected_cores;
}


void
Haruhi::session_loader()
{
	SessionLoader* loader = new SessionLoader (SessionLoader::NewTab, _session ? SessionLoader::CancelButton : SessionLoader::QuitButton, 0);
	if (loader->exec() == SessionLoader::Accepted)
	{
		delete _session;
		_session = new Session (0);
		_session->resize (600, 500);
		_session->show();
		loader->apply (_session);
	}
	else
		_session = 0;
	delete loader;
}


void
Haruhi::ok_to_quit()
{
	_ok_to_quit = true;
}


void
Haruhi::quit_if_ok()
{
	if (_ok_to_quit)
		_app->quit();
}

} // namespace Haruhi

