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
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QDialog>
#include <QtGui/QRadioButton>
#include <QtGui/QMessageBox>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QHeaderView>
#include <QtGui/QSplitter>
#include <QtGui/QMenu>
#include <Qt3Support/Q3ListView> // Required due to Qt4 bug in which normally QScrollBar is not QWidget.

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
	_context_menu (0),
	_highlight_connected (false)
{
	_splitter = new QSplitter (Qt::Horizontal, this);
	_splitter->setChildrenCollapsible (false);

	_opanel = new Panel (Port::Output, this, _splitter);
	_connector = new Connector (this, _splitter);
	_ipanel = new Panel (Port::Input, this, _splitter);

	_splitter->setStretchFactor (0, 4);
	_splitter->setStretchFactor (1, 3);
	_splitter->setStretchFactor (2, 4);

	_connect_button = new QPushButton (Resources::Icons16::connect(), "&Connect", this);
	_connect_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);

	_disconnect_button = new QPushButton (Resources::Icons16::disconnect(), "&Disconnect", this);
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

	QObject::connect (_connect_button, SIGNAL (clicked()), this, SLOT (connect_selected()));
	QObject::connect (_disconnect_button, SIGNAL (clicked()), this, SLOT (disconnect_selected()));

	// Layouts:

	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::Spacing);
	buttons_layout->addWidget (_connect_button);
	buttons_layout->addWidget (_disconnect_button);
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (_splitter);
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

	PortItem* port_item = dynamic_cast<PortItem*> (item);

	if (!_context_menu)
		_context_menu = new QMenu (this);
	else
		_context_menu->clear();

	QAction* connect_action = _context_menu->addAction (Resources::Icons16::connect(), "&Connect", this, SLOT (connect_selected()));
	connect_action->setEnabled (can_connect_selected());

	QAction* disconnect_action = _context_menu->addAction (Resources::Icons16::disconnect(), "&Disconnect", this, SLOT (disconnect_selected()));
	disconnect_action->setEnabled (can_disconnect_selected());

	QAction* disconnect_all_action = _context_menu->addAction (Resources::Icons16::disconnect(), "Disconnect all", this, SLOT (disconnect_all_from_selected()));
	disconnect_all_action->setEnabled (port_item &&
									   ((port_item->treeWidget() == _ipanel->list() && !port_item->port()->back_connections().empty()) ||
									    (port_item->treeWidget() == _opanel->list() && !port_item->port()->forward_connections().empty())));

	_context_menu->addSeparator();

	_context_item = item;
	_context_menu->exec (pos);
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
		PortsConnectorPrivate::HighlightableItem* hitem = dynamic_cast<PortsConnectorPrivate::HighlightableItem*> (item);
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
		PortItem* port_item = dynamic_cast<PortItem*> (_context_item);
		if (port_item)
		{
			_unit_bay->graph()->lock();
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
			_unit_bay->graph()->unlock();
		}
	}
}


void
PortsConnector::insert_unit (Unit* unit)
{
	_unit_bay->graph()->lock();
	unit_registered (unit);
	for (Ports::iterator p = unit->inputs().begin(); p != unit->inputs().end(); ++p)
		port_registered (*p, unit);
	for (Ports::iterator p = unit->outputs().begin(); p != unit->outputs().end(); ++p)
		port_registered (*p, unit);
	_unit_bay->graph()->unlock();
}


void
PortsConnector::remove_unit (Unit* unit)
{
	_unit_bay->graph()->lock();
	for (Ports::iterator p = unit->inputs().begin(); p != unit->inputs().end(); ++p)
		port_unregistered (*p, unit);
	for (Ports::iterator p = unit->outputs().begin(); p != unit->outputs().end(); ++p)
		port_unregistered (*p, unit);
	unit_unregistered (unit);
	_unit_bay->graph()->unlock();
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

	_unit_bay->graph()->lock();

	_ipanel->list()->read_units();
	_opanel->list()->read_units();
	_ipanel->filter()->read_units();
	_opanel->filter()->read_units();
	_connector->update();

	_unit_bay->graph()->unlock();
}


void
PortsConnector::remove_call_outs()
{
	for (CallOutEvents::size_type i = 0; i < _call_outs.size(); ++i)
		_call_outs[i]->cancel();
	_call_outs.clear();
}


template<class Port>
	inline void
	PortsConnector::operate_on_ports (Operation operation, Port* oport, Port* iport)
	{
		// Skip if ports are of incompatible types:
		if (oport && iport && typeid (*oport) != typeid (*iport))
			return;

		_unit_bay->graph()->lock();
		switch (operation)
		{
			case Private::Connect:		oport->connect_to (iport); break;
			case Private::Disconnect:	oport->disconnect_from (iport); break;
		}
		_unit_bay->graph()->unlock();
	}


void
PortsConnector::operate_on_selected (Operation operation)
{
	QTreeWidgetItem* oitem = _opanel->list()->selected_item();
	QTreeWidgetItem* iitem = _ipanel->list()->selected_item();

	bool (PortsConnector::*can_do_operation)(QTreeWidgetItem*, QTreeWidgetItem*) const;

	if (operation == Private::Connect)	can_do_operation = &PortsConnector::can_connect;
	else								can_do_operation = &PortsConnector::can_disconnect;

	if (oitem && iitem && (this->*can_do_operation)(oitem, iitem))
	{
		PortItem* po = dynamic_cast<PortItem*> (oitem);
		PortItem* pi = dynamic_cast<PortItem*> (iitem);
		UnitItem* uo = dynamic_cast<UnitItem*> (oitem);
		UnitItem* ui = dynamic_cast<UnitItem*> (iitem);

		// Port -> Port?
		if (po && pi)
			operate_on_ports (operation, po->port(), pi->port());
		// Port -> Unit?
		else if (po && ui)
		{
			Ports const& ports = ui->unit()->inputs();
			for (Ports::const_iterator port = ports.begin();  port != ports.end();  ++port)
				operate_on_ports (operation, po->port(), *port);
		}
		// Unit -> Port?
		else if (uo && pi)
		{
			Ports const& ports = uo->unit()->outputs();
			for (Ports::const_iterator port = ports.begin();  port != ports.end();  ++port)
				operate_on_ports (operation, *port, pi->port());
		}
		// Unit -> Unit?
		else if (uo && ui)
		{
			// Filter audio/event ports:
			std::list<Port*> audio_oports, audio_iports;
			std::list<Port*> event_oports, event_iports;
			for (Ports::const_iterator p = uo->unit()->outputs().begin();  p != uo->unit()->outputs().end();  ++p)
			{
				if (dynamic_cast<AudioPort*> (*p))
					audio_oports.push_back (*p);
				else if (dynamic_cast<EventPort*> (*p))
					event_oports.push_back (*p);
			}
			for (Ports::const_iterator p = ui->unit()->inputs().begin();  p != ui->unit()->inputs().end();  ++p)
			{
				if (dynamic_cast<AudioPort*> (*p))
					audio_iports.push_back (*p);
				else if (dynamic_cast<EventPort*> (*p))
					event_iports.push_back (*p);
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
	QTreeWidgetItem* oitem = _opanel->list()->selected_item();
	QTreeWidgetItem* iitem = _ipanel->list()->selected_item();
	return oitem && iitem && can_operate_on (operation, oitem, iitem);
}


bool
PortsConnector::can_operate_on (Operation operation, QTreeWidgetItem* oitem, QTreeWidgetItem* iitem) const
{
	PortItem* po = dynamic_cast<PortItem*> (oitem);
	PortItem* pi = dynamic_cast<PortItem*> (iitem);
	UnitItem* uo = dynamic_cast<UnitItem*> (oitem);
	UnitItem* ui = dynamic_cast<UnitItem*> (iitem);

	if (po && pi && typeid (*po->port()) != typeid (*pi->port()))
		return false;

	// TODO obsługa GroupItemów. Również w operate_on(…)
	if (operation == Private::Connect)
		return (po && pi && !po->port()->connected_to (pi->port()))
			|| (po && ui && ui->count_inputs_if (std::not1 (GraphDetail::connected_from (po->port()))) > 0)
			|| (uo && pi && uo->count_outputs_if (std::not1 (GraphDetail::connected_to (pi->port()))) > 0)
			|| (uo && ui); // TODO: && istnieją takie, które nie są połączone, a były by po połączeniu:
	else
		return (po && pi && po->port()->connected_to (pi->port()))
			|| (po && ui && ui->count_inputs_if (GraphDetail::connected_from (po->port())) > 0)
			|| (uo && pi && uo->count_outputs_if (GraphDetail::connected_to (pi->port())) > 0)
			|| (uo && ui); // TODO: && istnieją połączenia, które były by stworzone po połączeniu uo+ui
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
	for (PortItems::iterator pi = _highlighted_items.begin(); pi != _highlighted_items.end(); ++pi)
		(*pi)->set_highlighted (false);
	_highlighted_items.clear();

	QTreeWidgetItem* oitem = _opanel->list()->selected_item();
	QTreeWidgetItem* iitem = _ipanel->list()->selected_item();

	if (oitem)
	{
		// Highlight items in ipanel:
		PortItem* oportitem = dynamic_cast<PortItem*> (oitem);
		if (oportitem)
		{
			Ports const& iports = oportitem->port()->forward_connections();
			_unit_bay->graph()->lock();
			for (Ports::iterator p = iports.begin(); p != iports.end(); ++p)
			{
				PortItem* pi = find_port_item (*p);
				if (pi)
				{
					pi->set_highlighted (true);
					_highlighted_items.insert (pi);
				}
			}
			_unit_bay->graph()->unlock();
		}
	}

	if (iitem)
	{
		// Highlight items in opanel:
		PortItem* iportitem = dynamic_cast<PortItem*> (iitem);
		if (iportitem)
		{
			_unit_bay->graph()->lock();
			Ports const& oports = iportitem->port()->back_connections();
			for (Ports::iterator p = oports.begin(); p != oports.end(); ++p)
			{
				PortItem* pi = find_port_item (*p);
				if (pi)
				{
					pi->set_highlighted (true);
					_highlighted_items.insert (pi);
				}
			}
			_unit_bay->graph()->unlock();
		}
	}
}


Private::UnitItem*
PortsConnector::find_unit_item (Port::Direction direction, Unit* core_unit) const
{
	Unit* unit = dynamic_cast<Unit*> (core_unit);
	if (unit)
	{
		PortsList::UnitsToItemsMap const* units_list = 0;
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

