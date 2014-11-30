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

#ifndef HARUHI__SETTINGS__SESSION_LOADER_SETTINGS_H__INCLUDED
#define HARUHI__SETTINGS__SESSION_LOADER_SETTINGS_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>
#include <map>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "settings.h"


namespace Haruhi {

class SessionLoaderSettings: public Settings::Module
{
  public:
	/**
	 * Represents entry in a list of recent sessions
	 * shown in session manager.
	 */
	class RecentSession: public SaveableState
	{
	  public:
		RecentSession();

		RecentSession (QString const& name, QString const& file_name, Time timestamp);

		/*
		 * SaveableState API
		 */

		void
		save_state (QDomElement& element) const override;

		void
		load_state (QDomElement const& element) override;

		/*
		 * Predicates used by standard algorithms.
		 */

		static bool
		gt_by_timestamp (RecentSession const& a, RecentSession const& b);

		static bool
		lt_by_name (RecentSession const& a, RecentSession const& b);

		static bool
		lt_by_file_name_and_gt_by_timestamp (RecentSession const& a, RecentSession const& b);

		static bool
		eq_by_file_name (RecentSession const& a, RecentSession const& b);

		static bool
		file_not_exist (RecentSession const& a);

	  public:
		QString		name;
		QString		file_name;
		Time		timestamp;
	};

	typedef std::vector<RecentSession> RecentSessions;

  public:
	SessionLoaderSettings();

	void
	save_state (QDomElement& element) const;

	void
	load_state (QDomElement const& element);

	/**
	 * Removes duplicates and sessions with nonexistent files,
	 * limits number of recent sessions.
	 */
	void
	cleanup() const;

	RecentSessions&
	recent_sessions();

  private:
	RecentSessions mutable _recent_sessions;
};


inline SessionLoaderSettings::RecentSessions&
SessionLoaderSettings::recent_sessions()
{
	return _recent_sessions;
}

} // namespace Haruhi

#endif

