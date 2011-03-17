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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__MODEL_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__MODEL_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>
#include <map>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/signal.h>
#include <haruhi/utility/exception.h>
#include <haruhi/utility/mutex.h>

// Local:
#include "package.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

/**
 * Model for PresetsManager.
 * Model is associated with directory path, which is expected to contain
 * package files.
 */
class Model
{
	typedef std::map<Model*, int> ModelsByDir;

  public:
	class Locked: public Exception
	{
	  public:
		Locked (const char* details):
			Exception ("can't create presets model", details)
		{ }
	};

	typedef std::list<Package> Packages;

  public:
	/**
	 * Finds existing allocated model for given directory.
	 * If not found, creates new one.
	 */
	static Model*
	get (QString const& directory);

	/**
	 * Releases given model (deletes it if ref counter hits 0)
	 */
	static void
	release (Model* model);

	/**
	 * Upon creation model will create lock file in specified directory.
	 * If there is already lock file, throws Locked exception.
	 * \param	directory Directory path which contains package files.
	 */
	Model (QString const& directory);

	/**
	 * Removes lock file from associated directory.
	 */
	~Model();

	/**
	 * Returns associated directory, that is filesystem path
	 * to directory where package files (containing presets) are stored.
	 */
	QString const&
	directory() const { return _directory; }

	/**
	 * Packages accessor.
	 */
	Packages&
	packages() { return _packages; }

	/**
	 * Packages accessor.
	 */
	Packages const&
	packages() const { return _packages; }

	/**
	 * Creates new package. Does not save it.
	 */
	Package*
	create_package();

	/**
	 * Should be called when model changes, ie. packages/categories/presets are added or removed,
	 * or their properties change. Emits on_change() signal.
	 */
	void
	changed();

	/**
	 * Drops all current packages and rereads them from
	 * associated directory.
	 */
	void
	load_state();

	/**
	 * Saves all information to associated directory.
	 */
	void
	save_state();

  public:
	// Emited when model_changed() is called:
	Signal::Emiter0	on_change;

  private:
	/**
	 * Locks presets directory or throws the Locked exception.
	 */
	void
	acquire_lock();

	/**
	 * Releases lock from associated directory.
	 */
	void
	release_lock();

  private:
	Packages			_packages;
	QString				_directory;
	QString				_lock_file_name;
	// Lock file descriptor:
	int					_lock_file;
	// Registered models:
	static ModelsByDir	_models_by_dir;
	static Mutex		_models_by_dir_mutex;
};

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

