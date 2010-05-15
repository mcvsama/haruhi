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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__PORT_ITEM_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__PORT_ITEM_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "ports_list_view.h"


namespace Haruhi {

namespace EventBackendPrivate {

class InternalInputItem;

class PortItem:
	public QTreeWidgetItem,
	public SaveableState
{
  public:
	typedef std::set<InternalInputItem*> InternalInputs;

  public:
	PortItem (PortsListView* parent, QString const& name);

	PortItem (PortItem* parent, QString const& name);

	~PortItem();

	void
	update_minimum_size();

	InternalInputs*
	internal_inputs() { return &_internal_inputs; }

	/**
	 * Updates name of backend ports basing on GUI port name.
	 */
	virtual void
	update_name() = 0;

	/**
	 * Tells whether port has been fully constructed and
	 * may be normally used by backend.
	 */
	bool
	ready() const { return _ready; }

  protected:
	void
	set_ready (bool r) { atomic (_ready) = r; }

	PortItem*
	port_item() const { return _port_item; }

  protected:
	EventBackend*	_backend;
	// Link to parent PortItem or 0 if it's a chlid of TreeWidget.
	PortItem*		_port_item;
	// For quick traversal over children. Child items
	// will add/remove itself from this set:
	InternalInputs	_internal_inputs;

  private:
	// Set when port is fully constructed:
	bool			_ready;
};

} // namespace EventBackendPrivate

} // namespace Haruhi

#endif

