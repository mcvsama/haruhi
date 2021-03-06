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

#ifndef HARUHI__GRAPH__CONN_SET_H__INCLUDED
#define HARUHI__GRAPH__CONN_SET_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <list>
#include <set>

// Qt:
#include <QDomElement>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/graph/unit.h>


namespace Haruhi {

class ConnSet: public SaveableState
{
	class Connection
	{
	  public:
		Connection (QString const& source_unit, QString const& source_port, QString const& target_unit, QString const& target_port);

	  public:
		QString	source_unit;
		QString	source_port;
		QString	target_unit;
		QString	target_port;
	};

  public:
	typedef std::set<Unit*>			Units;
	typedef std::list<Connection>	Connections;

  public:
	/**
	 * \param	only_internal: Dump only connections between inserted units,
	 * 			and no other.
	 */
	ConnSet (bool only_internal = false);

	void
	insert_unit (Unit*);

	template<class UnitPointerIterator>
		void
		insert_units (UnitPointerIterator begin, UnitPointerIterator end);

	void
	remove_unit (Unit*);

	void
	save();

	void
	load() const;

	/*
	 * API of SaveableState
	 */

	void
	save_state (QDomElement&) const override;

	void
	load_state (QDomElement const&) override;

  private:
	Units		_units;
	Connections	_connections;
	bool		_only_internal;
};


inline
ConnSet::Connection::Connection (QString const& source_unit, QString const& source_port, QString const& target_unit, QString const& target_port):
	source_unit (source_unit),
	source_port (source_port),
	target_unit (target_unit),
	target_port (target_port)
{ }


template<class UnitPointerIterator>
	inline void
	ConnSet::insert_units (UnitPointerIterator begin, UnitPointerIterator end)
	{
		for (UnitPointerIterator u = begin; u != end; ++u)
			insert_unit (*u);
	}

} // namespace Haruhi

#endif

