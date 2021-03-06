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

#ifndef HARUHI__COMPONENTS__PORTS_CONNECTOR__PORTS_CONNECTOR_H__INCLUDED
#define HARUHI__COMPONENTS__PORTS_CONNECTOR__PORTS_CONNECTOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>
#include <map>
#include <set>

// Qt:
#include <QWidget>
#include <QSplitter>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QMenu>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/application/services.h>
#include <haruhi/graph/predicates.h>
#include <haruhi/session/unit_bay.h>
#include <haruhi/utility/signal.h>

// Local:
#include "connector.h"
#include "panel.h"
#include "ports_list.h"
#include "unit_item.h"
#include "comparable_item.h"
#include "units_combobox.h"


namespace Haruhi {

class PortsConnector;

namespace PortsConnectorPrivate {

	enum Operation { Connect, Disconnect };

}

class PortsConnector:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

	typedef std::set<Unit*> UnitsSet;
	typedef std::set<PortsConnectorPrivate::PortItem*> PortItems;
	typedef std::map<Port*, PortsConnectorPrivate::PortItem*> PortsToItemsMap;
	typedef std::vector<Services::CallOutEvent*> CallOutEvents;

  public:
	typedef PortsConnectorPrivate::Connector	Connector;
	typedef PortsConnectorPrivate::Panel		Panel;
	typedef PortsConnectorPrivate::PortsList	PortsList;
	typedef PortsConnectorPrivate::UnitItem		UnitItem;
	typedef PortsConnectorPrivate::GroupItem	GroupItem;
	typedef PortsConnectorPrivate::PortItem		PortItem;
	typedef PortsConnectorPrivate::Operation	Operation;

	friend class PortsConnectorPrivate::Connector;
	friend class PortsConnectorPrivate::Panel;
	friend class PortsConnectorPrivate::PortsList;
	friend class PortsConnectorPrivate::UnitItem;
	friend class PortsConnectorPrivate::GroupItem;
	friend class PortsConnectorPrivate::PortItem;
	friend class PortsConnectorPrivate::UnitsCombobox;

  public:
	PortsConnector (UnitBay* unit_bay, QWidget* parent = 0);

	virtual ~PortsConnector();

	UnitBay*
	unit_bay() const;

	PortsConnectorPrivate::Connector*
	connector() const;

	/**
	 * This should be called by deleted item in its destructor so
	 * Connector will remove all cached information related to the item
	 * (eg. remove from highlighted-items list).
	 */
	void
	forget_item (PortItem*);

	/**
	 * Adds Unit that will be always inserted into list.
	 * Useful for Audio/Event backend units, etc.
	 */
	void
	add_external_unit (Unit*);

  public slots:
	void
	context_menu (QTreeWidgetItem* item, QPoint const& pos);

	void
	list_view_moved();

	void
	list_view_changed();

	void
	item_expanded_or_collapsed (QTreeWidgetItem*);

	void
	selection_changed();

	void
	connect_selected();

	void
	disconnect_selected();

	void
	disconnect_all_from_selected();

	void
	insert_unit (Unit*);

	void
	remove_unit (Unit*);

  private:
	/*
	 * Graph notification slots (they only send callback
	 * events to Qt event queue).
	 */

	void
	unit_registered (Unit*);

	void
	unit_unregistered (Unit*);

	void
	unit_retitled (Unit*);

	void
	port_renamed (Port*);

	void
	port_connected_to (Port*, Port*);

	void
	port_disconnected_from (Port*, Port*);

	void
	port_registered (Port*, Unit*);

	void
	port_unregistered (Port*, Unit*);

	void
	port_group_renamed (PortGroup*);

	/**
	 * Actually updates widgets. Should be called
	 * from withing Qt thread.
	 */
	void
	graph_changed();

	/**
	 * Remove all created callouts.
	 */
	void
	remove_call_outs();

  private:
	template<class Port>
		inline void
		operate_on_ports (PortsConnectorPrivate::Operation operation, Port* oport, Port* iport);

	void
	operate_on_selected (PortsConnectorPrivate::Operation);

	bool
	can_operate_on_selected (PortsConnectorPrivate::Operation) const;

	bool
	can_operate_on (PortsConnectorPrivate::Operation, QTreeWidgetItem* oitem, QTreeWidgetItem* iitem) const;

	bool
	can_connect_selected() const;

	bool
	can_disconnect_selected() const;

	bool
	can_connect (QTreeWidgetItem* oitem, QTreeWidgetItem* iitem) const;

	bool
	can_disconnect (QTreeWidgetItem* oitem, QTreeWidgetItem* iitem) const;

	void
	highlight_connected();

	UnitItem*
	find_unit_item (Port::Direction direction, Unit*) const;

	PortItem*
	find_port_item (Port* port) const;

	/**
	 * Returns true if unit is registered in Graph.
	 */
	bool
	validate_unit (Unit*);

	/**
	 * Returns true if unit is registered in Graph and
	 * port belongs to that unit.
	 */
	bool
	validate_unit_and_port (Unit*, Port*);

  private:
	UnitsSet									_external_units;		// External Units to UnitBay that are included in lists.
	UnitBay*									_unit_bay;
	Unique<QSplitter>							_splitter;
	Unique<PortsConnectorPrivate::Panel>		_opanel;
	Unique<PortsConnectorPrivate::Panel>		_ipanel;
	Unique<PortsConnectorPrivate::Connector>	_connector;
	Unique<QPushButton>							_connect_button;
	Unique<QPushButton>							_disconnect_button;
	QTreeWidgetItem*							_context_item;
	PortsToItemsMap								_ports_to_items;		// Maps ports to items in lists.
	PortItems									_highlighted_items;		// Currently highlighted items set.
	bool										_highlight_connected;	// Perform highlighting?
	CallOutEvents								_call_outs;
};


inline UnitBay*
PortsConnector::unit_bay() const
{
	return _unit_bay;
}


inline PortsConnectorPrivate::Connector*
PortsConnector::connector() const
{
	return _connector.get();
}

} // namespace Haruhi

#endif

