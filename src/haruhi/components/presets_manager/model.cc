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

// Standard:
#include <cstddef>

// System:
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

// Qt:
#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QtGui/QTextDocument>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/filesystem.h>
#include <haruhi/utility/lexical_cast.h>
#include <haruhi/utility/predicates.h>

// Local:
#include "model.h"
#include "package.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

Model::ModelsByDir	Model::_models_by_dir;
Mutex				Model::_models_by_dir_mutex;


Model*
Model::get (QString const& directory)
{
	_models_by_dir_mutex.lock();

	Model* res = 0;
	ModelsByDir::iterator m;

	for (m = _models_by_dir.begin(); m != _models_by_dir.end(); ++m)
		if (m->first->directory() == directory)
			break;

	if (m != _models_by_dir.end())
	{
		m->second += 1;
		res = m->first;
	}
	else
	{
		res = new Model (directory);
		res->load_state();
		_models_by_dir[res] = 1;
	}

	_models_by_dir_mutex.unlock();
	return res;
}


void
Model::release (Model* model)
{
	_models_by_dir_mutex.lock();

	ModelsByDir::iterator m = _models_by_dir.find (model);
	assert (m != _models_by_dir.end());

	m->second -= 1;
	if (m->second == 0)
	{
		delete model;
		_models_by_dir.erase (model);
	}

	_models_by_dir_mutex.unlock();
}


Model::Model (QString const& directory):
	_directory (directory),
	_lock_file_name (directory + "/" + ".haruhi-lock.pid")
{
	acquire_lock();
}


Model::~Model()
{
	release_lock();
}


Package*
Model::create_package()
{
	_packages.push_back (Package());
	return &_packages.back();
}


void
Model::remove_package (Package* package)
{
	_packages.remove_if (PointerEquals<Package> (package));
}


void
Model::changed()
{
	on_change();
}


void
Model::load_state()
{
	_packages.clear();
	// Read packages:
	QDir dir (directory(), "*.haruhi-presets");
	QStringList list = dir.entryList();
	// Each entry in list represents one presets package:
	for (QStringList::Iterator p = list.begin(); p != list.end(); ++p)
	{
		Package package;
		package.load_file (directory() + "/" + *p);
		_packages.push_back (package);
	}
}


void
Model::save_state()
{
	for (Packages::iterator p = _packages.begin(); p != _packages.end(); ++p)
		p->save_file (directory() + "/" + p->file_name());
}


void
Model::acquire_lock()
{
	mkpath (directory().toStdString(), 0700);

	for (int t = 0; t < 2; ++t)
	{
		_lock_file = ::open (_lock_file_name, O_CREAT | O_EXCL | O_WRONLY, 0644);
		// If lock is acquired:
		if (_lock_file != -1)
		{
			// Write our pid to file:
			QString pid = QString ("%1").arg (::getpid());
			if (::write (_lock_file, pid.ascii(), strlen (pid.ascii())) == -1)
				throw Locked ("failed to write Mikuru's PID to the lock file");
			break;
		}
		else
		{
			// Check if process that acquired lock still exist:
			_lock_file = ::open (_lock_file_name, O_EXCL | O_RDONLY);
			if (_lock_file == -1)
				throw Locked (QString ("failed to obtain lock '%1'"));
			// Read PID:
			char buf[16] = {0};
			read (_lock_file, buf, 15);
			int pid = lexical_cast<int> (buf);
			// Check if other process exist:
			if (pid != 0 && kill (pid, 0) != 0)
				unlink (_lock_file_name);
			else
				throw Locked (QString ("locked by process %1").arg (pid).utf8());
		}
	}
}


void
Model::release_lock()
{
	if (_lock_file != -1)
		::unlink (_lock_file_name);
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

