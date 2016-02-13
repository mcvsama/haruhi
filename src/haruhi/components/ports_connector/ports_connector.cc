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
 * --
 * Drag/drop stuff and curves rendering has been imported from well known
 * Qjackctl program by rncbc aka Rui Nuno Capela.
 */

// Standard:
#include <cstddef>
#include <utility>
#include <iterator>
#include <typeinfo>
#include <algorithm>
#include <memory>
#include <list>
#include <map>

// Qt:
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QScrollBar>
#include <QMenu>
#include <QHeaderView>
#include <QLayout>

// Lib:
#include <boost/bind/bind.hpp>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/application/services.h>
#include <haruhi/graph/audio_port.h>
#include <haruhi/graph/event_port.h>

// Local:
#include "ports_connector.h"
#include "highlightable_item.h"


namespace Haruhi {

namespace Private = PortsConnectorPrivate;

PortsConnector::PortsConnector (UnitBay* unit_bay, QWidget* parent):
	QWidget (parent),
	_unit_bay (unit_bay),
	_context_item (0),
	_highlight_connected (false)
{
	_splitter = std::make_unique<QSplitter> (Qt::Horizontal, this);
	_splitter->setChildrenCollapsible (false);

	_opanel = std::make_unique<Panel> (Port::Output, this, _splitter.get());
	_connector = std::make_unique<Connector> (this, _splitter.get());
	_ipanel = std::make_unique<Panel> (Port::Input, this, _splitter.get());

	_splitter->setStretchFactor (0, 4);
	_splitter->setStretchFactor (1, 3);
	_splitter->setStretchFactor (2, 4);

	_connect_button = std::make_unique<QPushButton> (Resources::Icons16::connect(), "&Connect", this);
	_connect_button->setIconSize (Resources::Icons16::haruhi().size());
	_connect_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);

	_disconnect_button = std::make_unique<QPushButton> (Resources::Icons16::disconnect(), "&Disconnect", this);
	_disconnect_button->setIconSize (Resources::Icons16::haruhi().size());
	_disconnect_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);

	QObject::connect (_ipanel->list()->verticalScrollBar(), SIGNAL (valueChanged (int)), static_cast<QWidget*> (this), SLOT (list_view_moved()));
	QObject::connect (_ipanel->list()->header(), SIGNAL (sectionClicked (int)), this, SLOT (list_view_moved()));
	QObject::connect (_ipanel->list(), SIGNAL (itemExpanded (QTreeWidgetItem*)), this, SLOT (list_view_changed()));
	QObject::connect (_ipanel->list(), SIGNAL (itemCollapsed (QTreeWidgetItem*)), this, SLOT (list_view_changed()));
	QObject::connect (_ipanel->list(), SIGNAL (itemSelectionChanged()), this, SLOT (selection_changed()));
	QObject::connect (_ipanel->list(), SIGNAL (context_menu (QTreeWidgetItem*, const QPoint&)), this, SLOT (context_menu (QTreeWidgetItem*, const QPoint&)));
	QObject::connect (_ipanel->list(), SIGNAL (itemExpanded (QTreeWidgetItem*)), this, SLOT (item_expanded_or_collapsed (QTreeWidgetItem*)));
	QObject::connect (_ipanel->list(), SIGNAL (itemCollapsed (QTreeWidgetItem*)), this, SLOT (item_expanded_or_collapsed (QTreeWidgetItem*)));

	QObject::connect (_opanel->list()->verticalScrollBar(), SIGNAL (valueChanged (int)), this, SLOT (list_view_moved()));
	QObject::connect (_opanel->list()->header(), SIGNAL (sectionClicked (int)), this, SLOT (list_view_moved()));
	QObject::connect (_opanel->list(), SIGNAL (itemExpanded (QTreeWidgetItem*)), this, SLOT (list_view_changed()));
	QObject::connect (_opanel->list(), SIGNAL (itemCollapsed (QTreeWidgetItem*)), this, SLOT (list_view_changed()));
	QObject::connect (_opanel->list(), SIGNAL (itemSelectionChanged()), this, SLOT (selection_changed()));
	QObject::connect (_opanel->list(), SIGNAL (context_menu (QTreeWidgetItem*, const QPoint&)), this, SLOT (context_menu (QTreeWidgetItem*, const QPoint&)));
	QObject::connect (_opanel->list(), SIGNAL (itemExpanded (QTreeWidgetItem*)), this, SLOT (item_expanded_or_collapsed (QTreeWidgetItem*)));
	QObject::connect (_opanel->list(), SIGNAL (itemCollapsed (QTreeWidgetItem*)), this, SLOT (item_expanded_or_collapsed (QTreeWidgetItem*)));

	QObject::connect (_connect_button.get(), SIGNAL (clicked()), this, SLOT (connect_selected()));
	QObject::connect (_disconnect_button.get(), SIGNAL (clicked()), this, SLOT (disconnect_selected()));

	// Layouts:

	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	auto buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::spacing());
	buttons_layout->addWidget (_connect_button.get());
	buttons_layout->addWidget (_disconnect_button.get());
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	auto layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addWidget (_splitter.get());
	layout->addLayout (buttons_layout);

	_unit_bay->graph()->unit_registered.connect (this, &PortsConnector::unit_registered);
	_unit_bay->graph()->unit_unregistered.connect (this, &PortsConnector::unit_unregistered);
	_unit_bay->graph()->unit_retitled.connect (this, &PortsConnector::unit_retitled);
	_unit_bay->graph()->port_renamed.connect (this, &PortsConnector::port_renamed);
	_unit_bay->graph()->port_connected_to.connect (this, &PortsConnector::port_connected_to);
	_unit_bay->graph()->port_disconnected_from.connect (this, &PortsConnector::port_disconnected_from);
	_unit_bay->graph()->port_registered.connect (this, &PortsConnector::port_registered);
	_unit_bay->graph()->port_unregistered.connect (this, &PortsConnector::port_unregistered);
	_unit_bay->graph()->port_group_renamed.connect (this, &PortsConnector::port_group_renamed);

	insert_unit (_unit_bay);

	_highlight_connected = true;
}


PortsConnector::~PortsConnector()
{
	Signal::Receiver::disconnect_all_signals();

	_highlighted_items.clear();
	_highlight_connected = false;

	_unit_bay->graph()->unit_registered.disconnect (this, &PortsConnector::unit_registered);
	_unit_bay->graph()->unit_unregistered.disconnect (this, &PortsConnector::unit_unregistered);
	_unit_bay->graph()->unit_retitled.disconnect (this, &PortsConnector::unit_retitled);
	_unit_bay->graph()->port_renamed.disconnect (this, &PortsConnector::port_renamed);
	_unit_bay->graph()->port_connected_to.disconnect (this, &PortsConnector::port_connected_to);
	_unit_bay->graph()->port_disconnected_from.disconnect (this, &PortsConnector::port_disconnected_from);
	_unit_bay->graph()->port_registered.disconnect (this, &PortsConnector::port_registered);
	_unit_bay->graph()->port_unregistered.disconnect (this, &PortsConnector::port_unregistered);
	_unit_bay->graph()->port_group_renamed.disconnect (this, &PortsConnector::port_group_renamed);

	_ipanel->list()->clear();
	_opanel->list()->clear();
}


void
PortsConnector::forget_item (PortItem* item)
{
	_highlighted_items.erase (item);
}


void
PortsConnector::add_external_unit (Unit* unit)
{
	_external_units.insert (unit);
	insert_unit (unit);
}


void
PortsConnector::context_menu (QTreeWidgetItem* item, QPoint const& pos)
{
	if (!item)
		return;

	auto port_item = dynamic_cast<PortItem*> (item);
	auto menu = std::make_unique<QMenu> (this);

	auto a = menu->addAction (Resources::Icons16::connect(), "&Connect", this, SLOT (connect_selected()));
	a->setEnabled (can_connect_selected());

	a = menu->addAction (Resources::Icons16::disconnect(), "&Disconnect", this, SLOT (disconnect_selected()));
	a->setEnabled (can_disconnect_selected());

	a = menu->addAction (Resources::Icons16::disconnect(), "Disconnect all", this, SLOT (disconnect_all_from_selected()));
	a->setEnabled (port_item &&
				   ((port_item->treeWidget() == _ipanel->list() && !port_item->port()->back_connections().empty()) ||
					(port_item->treeWidget() == _opanel->list() && !port_item->port()->forward_connections().empty())));

	_context_item = item;
	menu->exec (pos);
}


void
PortsConnector::list_view_moved()
{
	selection_changed();
}


void
PortsConnector::list_view_changed()
{
	selection_changed();
}


void
PortsConnector::item_expanded_or_collapsed (QTreeWidgetItem* item)
{
	if (item)
	{
		auto hitem = dynamic_cast<PortsConnectorPrivate::HighlightableItem*> (item);
		if (hitem)
			hitem->update_highlight();
	}
}


void
PortsConnector::selection_changed()
{
	_connect_button->setEnabled (can_connect_selected());
	_disconnect_button->setEnabled (can_disconnect_selected());
	highlight_connected();
	_connector->update();
}


void
PortsConnector::connect_selected()
{
	operate_on_selected (Private::Connect);
}

void
PortsConnector::disconnect_selected()
{
	operate_on_selected (Private::Disconnect);
}


void
PortsConnector::disconnect_all_from_selected()
{
	if (_context_item)
	{
		auto port_item = dynamic_cast<PortItem*> (_context_item);
		if (port_item)
		{
			Mutex::Lock lock (*_unit_bay->graph());
			if (port_item->treeWidget() == _ipanel->list())
			{
				while (!port_item->port()->back_connections().empty())
					(*port_item->port()->back_connections().begin())->disconnect_from (port_item->port());
			}
			else if (port_item->treeWidget() == _opanel->list())
			{
				while (!port_item->port()->forward_connections().empty())
					port_item->port()->disconnect_from (*port_item->port()->forward_connections().begin());
			}
		}
	}
}


void
PortsConnector::insert_unit (Unit* unit)
{
	Mutex::Lock lock (*_unit_bay->graph());
	unit_registered (unit);
	for (Port* p: unit->inputs())
		port_registered (p, unit);
	for (Port* p: unit->outputs())
		port_registered (p, unit);
}


void
PortsConnector::remove_unit (Unit* unit)
{
	Mutex::Lock lock (*_unit_bay->graph());
	for (Port* p: unit->inputs())
		port_unregistered (p, unit);
	for (Port* p: unit->outputs())
		port_unregistered (p, unit);
	unit_unregistered (unit);
}


void
PortsConnector::unit_registered (Unit*)
{
	_call_outs.push_back (Services::call_out (boost::bind (&PortsConnector::graph_changed, this)));
}


void
PortsConnector::unit_unregistered (Unit* unit)
{
	_call_outs.push_back (Services::call_out (boost::bind (&PortsConnector::graph_changed, this)));
	if (_external_units.find (unit) != _external_units.end())
		_external_units.erase (unit);
}


void
PortsConnector::unit_retitled (Unit*)
{
	_call_outs.push_back (Services::call_out (boost::bind (&PortsConnector::graph_changed, this)));
}


void
PortsConnector::port_renamed (Port*)
{
	_call_outs.push_back (Services::call_out (boost::bind (&PortsConnector::graph_changed, this)));
}


void
PortsConnector::port_connected_to (Port*, Port*)
{
	_call_outs.push_back (Services::call_out (boost::bind (&PortsConnector::graph_changed, this)));
}


void
PortsConnector::port_disconnected_from (Port*, Port*)
{
	_call_outs.push_back (Services::call_out (boost::bind (&PortsConnector::graph_changed, this)));
}


void
PortsConnector::port_registered (Port*, Unit*)
{
	_call_outs.push_back (Services::call_out (boost::bind (&PortsConnector::graph_changed, this)));
}


void
PortsConnector::port_unregistered (Port*, Unit*)
{
	_call_outs.push_back (Services::call_out (boost::bind (&PortsConnector::graph_changed, this)));
}


void
PortsConnector::port_group_renamed (PortGroup*)
{
	_call_outs.push_back (Services::call_out (boost::bind (&PortsConnector::graph_changed, this)));
}


void
PortsConnector::graph_changed()
{
	remove_call_outs();
	Mutex::Lock lock (*_unit_bay->graph());
	_ipanel->list()->read_units();
	_opanel->list()->read_units();
	_ipanel->filter()->read_units();
	_opanel->filter()->read_units();
	_connector->update();
}


void
PortsConnector::remove_call_outs()
{
	for (auto* ce: _call_outs)
		ce->cancel();
	_call_outs.clear();
}


template<class Port>
	inline void
	PortsConnector::operate_on_ports (Operation operation, Port* oport, Port* iport)
	{
		// Skip if ports are of incompatible types:
		if (oport && iport && typeid (*oport) != typeid (*iport))
			return;

		Mutex::Lock lock (*_unit_bay->graph());
		switch (operation)
		{
			case Private::Connect:		oport->connect_to (iport); break;
			case Private::Disconnect:	oport->disconnect_from (iport); break;
		}
	}


void
PortsConnector::operate_on_selected (Operation operation)
{
	auto oitem = _opanel->list()->selected_item();
	auto iitem = _ipanel->list()->selected_item();

	bool (PortsConnector::*can_do_operation)(QTreeWidgetItem*, QTreeWidgetItem*) const;

	if (operation == Private::Connect)
		can_do_operation = &PortsConnector::can_connect;
	else
		can_do_operation = &PortsConnector::can_disconnect;

	if (oitem && iitem && (this->*can_do_operation)(oitem, iitem))
	{
		auto po = dynamic_cast<PortItem*> (oitem);
		auto pi = dynamic_cast<PortItem*> (iitem);
		auto uo = dynamic_cast<UnitItem*> (oitem);
		auto ui = dynamic_cast<UnitItem*> (iitem);

		// Port -> Port?
		if (po && pi)
			operate_on_ports (operation, po->port(), pi->port());
		// Port -> Unit?
		else if (po && ui)
		{
			for (Port* p: ui->unit()->inputs())
				operate_on_ports (operation, po->port(), p);
		}
		// Unit -> Port?
		else if (uo && pi)
		{
			for (Port* p: uo->unit()->outputs())
				operate_on_ports (operation, p, pi->port());
		}
		// Unit -> Unit?
		else if (uo && ui)
		{
			// Filter audio/event ports:
			std::list<Port*> audio_oports, audio_iports;
			std::list<Port*> event_oports, event_iports;
			for (Port* p: uo->unit()->outputs())
			{
				if (dynamic_cast<AudioPort*> (p))
					audio_oports.push_back (p);
				else if (dynamic_cast<EventPort*> (p))
					event_oports.push_back (p);
			}
			for (Port* p: ui->unit()->inputs())
			{
				if (dynamic_cast<AudioPort*> (p))
					audio_iports.push_back (p);
				else if (dynamic_cast<EventPort*> (p))
					event_iports.push_back (p);
			}
			// Audio:
			{
				std::list<Port*>::const_iterator o = audio_oports.begin();
				std::list<Port*>::const_iterator i = audio_iports.begin();
				for (; o != audio_oports.end() && i != audio_iports.end();  ++o, ++i)
					operate_on_ports (operation, *o, *i);
			}
			// Event:
			{
				std::list<Port*>::const_iterator o = event_oports.begin();
				std::list<Port*>::const_iterator i = event_iports.begin();
				for (; o != event_oports.end() && i != event_iports.end();  ++o, ++i)
					operate_on_ports (operation, *o, *i);
			}
		}

		selection_changed();
	}
}


bool
PortsConnector::can_operate_on_selected (Operation operation) const
{
	auto oitem = _opanel->list()->selected_item();
	auto iitem = _ipanel->list()->selected_item();
	return oitem && iitem && can_operate_on (operation, oitem, iitem);
}


bool
PortsConnector::can_operate_on (Operation operation, QTreeWidgetItem* oitem, QTreeWidgetItem* iitem) const
{
	auto po = dynamic_cast<PortItem*> (oitem);
	auto pi = dynamic_cast<PortItem*> (iitem);
	auto uo = dynamic_cast<UnitItem*> (oitem);
	auto ui = dynamic_cast<UnitItem*> (iitem);

	if (po && pi && typeid (*po->port()) != typeid (*pi->port()))
		return false;

	// TODO handle GroupItems. Also in operate_on(…)
	if (operation == Private::Connect)
		return (po && pi && !po->port()->connected_to (pi->port()))
			|| (po && ui && ui->count_inputs_if (std::not1 (GraphDetail::connected_from (po->port()))) > 0)
			|| (uo && pi && uo->count_outputs_if (std::not1 (GraphDetail::connected_to (pi->port()))) > 0)
			|| (uo && ui); // TODO: && exist such ports which are not connected, but would be connected after… connecting.
	else
		return (po && pi && po->port()->connected_to (pi->port()))
			|| (po && ui && ui->count_inputs_if (GraphDetail::connected_from (po->port())) > 0)
			|| (uo && pi && uo->count_outputs_if (GraphDetail::connected_to (pi->port())) > 0)
			|| (uo && ui); // TODO: && there are connections that could be created after connecting uo + ui
}


bool
PortsConnector::can_connect_selected() const
{
	return can_operate_on_selected (Private::Connect);
}


bool
PortsConnector::can_disconnect_selected() const
{
	return can_operate_on_selected (Private::Disconnect);
}


bool
PortsConnector::can_connect (QTreeWidgetItem* oitem, QTreeWidgetItem* iitem) const
{
	return can_operate_on (Private::Connect, oitem, iitem);
}


bool
PortsConnector::can_disconnect (QTreeWidgetItem* oitem, QTreeWidgetItem* iitem) const
{
	return can_operate_on (Private::Disconnect, oitem, iitem);
}


void
PortsConnector::highlight_connected()
{
	if (!_highlight_connected)
		return;

	// Unhighlight all previous items:
	for (PortItem* pi: _highlighted_items)
		pi->set_highlighted (false);
	_highlighted_items.clear();

	auto oitem = _opanel->list()->selected_item();
	auto iitem = _ipanel->list()->selected_item();

	if (oitem)
	{
		// Highlight items in ipanel:
		auto oportitem = dynamic_cast<PortItem*> (oitem);
		if (oportitem)
		{
			Mutex::Lock lock (*_unit_bay->graph());
			for (Port* p: oportitem->port()->forward_connections())
			{
				auto pi = find_port_item (p);
				if (pi)
				{
					pi->set_highlighted (true);
					_highlighted_items.insert (pi);
				}
			}
		}
	}

	if (iitem)
	{
		// Highlight items in opanel:
		auto iportitem = dynamic_cast<PortItem*> (iitem);
		if (iportitem)
		{
			Mutex::Lock lock (*_unit_bay->graph());
			for (Port* p: iportitem->port()->back_connections())
			{
				auto pi = find_port_item (p);
				if (pi)
				{
					pi->set_highlighted (true);
					_highlighted_items.insert (pi);
				}
			}
		}
	}
}


Private::UnitItem*
PortsConnector::find_unit_item (Port::Direction direction, Unit* core_unit) const
{
	auto unit = dynamic_cast<Unit*> (core_unit);
	if (unit)
	{
		PortsList::UnitsToItemsMap const* units_list = nullptr;
		switch (direction)
		{
			case Port::Input:	units_list = _ipanel->list()->units(); break;
			case Port::Output:	units_list = _opanel->list()->units(); break;
		}
		PortsList::UnitsToItemsMap::const_iterator unit_item = units_list->find (unit);
		if (unit_item != units_list->end())
			return unit_item->second;
	}
	return 0;
}


Private::PortItem*
PortsConnector::find_port_item (Port* port) const
{
	PortsToItemsMap::const_iterator p = _ports_to_items.find (port);
	if (p != _ports_to_items.end())
		return p->second;
	return 0;
}


bool
PortsConnector::validate_unit (Unit* unit)
{
	return _unit_bay->graph()->units().find (unit) != _unit_bay->graph()->units().end();
}


bool
PortsConnector::validate_unit_and_port (Unit* unit, Port* port)
{
	return validate_unit (unit) &&
		(unit->inputs().find (port) != unit->inputs().end() || unit->outputs().find (port) != unit->outputs().end());
}

} // namespace Haruhi

