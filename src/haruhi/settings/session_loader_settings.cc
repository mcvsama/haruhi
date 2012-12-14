/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <algorithm>

// Local:
#include "session_loader_settings.h"

// Haruhi:
#include <haruhi/utility/qdom_sequence.h>


namespace Haruhi {

SessionLoaderSettings::RecentSession::RecentSession():
	name(),
	file_name(),
	timestamp()
{
}


SessionLoaderSettings::RecentSession::RecentSession (QString const& name, QString const& file_name, Timestamp timestamp):
	name (name),
	file_name (file_name),
	timestamp (timestamp)
{
}


void
SessionLoaderSettings::RecentSession::save_state (QDomElement& element) const
{
	element.setAttribute ("name", name);
	element.setAttribute ("file-name", file_name);
	element.setAttribute ("timestamp", static_cast<qlonglong> (timestamp.microseconds()));
}


void
SessionLoaderSettings::RecentSession::load_state (QDomElement const& element)
{
	name = element.attribute ("name", "<unknown name>");
	file_name = element.attribute ("file-name", "");
	timestamp.set_epoch_microseconds (element.attribute ("timestamp", "0").toInt());
}


bool
SessionLoaderSettings::RecentSession::gt_by_timestamp (RecentSession const& a, RecentSession const& b)
{
	return a.timestamp > b.timestamp;
}


bool
SessionLoaderSettings::RecentSession::lt_by_name (RecentSession const& a, RecentSession const& b)
{
	return a.name < b.name;
}


bool
SessionLoaderSettings::RecentSession::lt_by_file_name_and_gt_by_timestamp (RecentSession const& a, RecentSession const& b)
{
	return std::make_pair (a.file_name, -a.timestamp) < std::make_pair (b.file_name, -b.timestamp);
}


bool
SessionLoaderSettings::RecentSession::eq_by_file_name (RecentSession const& a, RecentSession const& b)
{
	return a.file_name == b.file_name;

}

bool
SessionLoaderSettings::RecentSession::file_not_exist (RecentSession const& a)
{
	return !QFile::exists (a.file_name);
}


SessionLoaderSettings::SessionLoaderSettings():
	Module ("session-loader")
{
}


void
SessionLoaderSettings::save_state (QDomElement& element) const
{
	cleanup();

	QDomElement recent_sessions_el = element.ownerDocument().createElement ("recent-sessions");
	element.appendChild (recent_sessions_el);

	for (auto& rs: _recent_sessions)
	{
		QDomElement rs_el = element.ownerDocument().createElement ("recent-session");
		rs.save_state (rs_el);
		recent_sessions_el.appendChild (rs_el);
	}
}


void
SessionLoaderSettings::load_state (QDomElement const& element)
{
	_recent_sessions.clear();

	for (QDomElement& e: element)
	{
		if (e.tagName() == "recent-sessions")
		{
			for (QDomElement& e2: e)
			{
				if (e2.tagName() == "recent-session")
				{
					RecentSession rs;
					rs.load_state (e2);
					_recent_sessions.push_back (rs);
				}
			}
		}
	}

	cleanup();
}


void
SessionLoaderSettings::cleanup() const
{
	RecentSessions::iterator rs;
	// Remove non-existent files:
	rs = std::remove_if (_recent_sessions.begin(), _recent_sessions.end(), RecentSession::file_not_exist);
	_recent_sessions.erase (rs, _recent_sessions.end());
	// Remove duplicates:
	std::sort (_recent_sessions.begin(), _recent_sessions.end(), RecentSession::lt_by_file_name_and_gt_by_timestamp);
	rs = std::unique (_recent_sessions.begin(), _recent_sessions.end(), RecentSession::eq_by_file_name);
	_recent_sessions.erase (rs, _recent_sessions.end());
	// Limit recent sessions entries:
	std::sort (_recent_sessions.begin(), _recent_sessions.end(), RecentSession::gt_by_timestamp);
	_recent_sessions.resize (std::min (static_cast<RecentSessions::difference_type> (_recent_sessions.size()),
									   static_cast<RecentSessions::difference_type> (32)));
	std::sort (_recent_sessions.begin(), _recent_sessions.end(), RecentSession::lt_by_name);
}

} // namespace Haruhi
