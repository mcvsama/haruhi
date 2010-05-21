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
#include <QtGui/QPainter>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QDialog>
#include <QtGui/QRadioButton>
#include <QtGui/QMessageBox>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QHeaderView>
#include <QtGui/QSplitter>
#include <QtGui/QMenu>
#include <Qt3Support/Q3DragObject>
#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3ButtonGroup>
#include <Qt3Support/Q3ListView> // Required due to Qt4 bug in which normally QScrollBar is not QWidget.

// Haruhi:
#include <haruhi/core/audio_buffer.h>
#include <haruhi/core/event_buffer.h>
#include <haruhi/config.h>
#include <haruhi/session.h>

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
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_layout = new QVBoxLayout (this, 0, Config::spacing);

		_splitter = new QSplitter (Qt::Horizontal, this);
		_splitter->setChildrenCollapsible (false);

			_opanel = new Panel (Core::Port::Output, this, _splitter);
			_connector = new Connector (this, _splitter);
			_ipanel = new Panel (Core::Port::Input, this, _splitter);

		_splitter->setStretchFactor (0, 4);
		_splitter->setStretchFactor (1, 3);
		_splitter->setStretchFactor (2, 4);

	_layout->addWidget (_splitter);

		QHBoxLayout* buttons_layout = new QHBoxLayout (_layout, Config::spacing);

		_connect_button = new QPushButton (Config::Icons16::connect(), "&Connect", this);
		_connect_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);

		_disconnect_button = new QPushButton (Config::Icons16::disconnect(), "&Disconnect", this);
		_disconnect_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);

		buttons_layout->addWidget (_connect_button);
		buttons_layout->addWidget (_disconnect_button);
		buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

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

	_unit_bay->graph()->unit_registered.connect (this, &PortsConnector::unit_registered);
	_unit_bay->graph()->unit_unregistered.connect (this, &PortsConnector::unit_unregistered);
	_unit_bay->graph()->unit_retitled.connect (this, &PortsConnector::unit_retitled);
	_unit_bay->graph()->port_renamed.connect (this, &PortsConnector::port_renamed);
	_unit_bay->graph()->port_connected_to.connect (this, &PortsConnector::port_connected_to);
	_unit_bay->graph()->port_disconnected_from.connect (this, &PortsConnector::port_disconnected_from);
	_unit_bay->graph()->port_registered.connect (this, &PortsConnector::port_registered);
	_unit_bay->graph()->port_unregistered.connect (this, &PortsConnector::port_unregistered);
	_unit_bay->graph()->port_group_renamed.connect (this, &PortsConnector::port_group_renamed);

	_external_units.insert (_unit_bay->session()->audio_backend());
	_external_units.insert (_unit_bay->session()->event_backend());

	insert_unit (_unit_bay);
	insert_unit (_unit_bay->session()->audio_backend());
	insert_unit (_unit_bay->session()->event_backend());

	_highlight_connected = true;
}


PortsConnector::~PortsConnector()
{
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
PortsConnector::create_port()
{
	Private::CreatePortDialog* dialog = new Private::CreatePortDialog (this);

	try {
		UnitItem* unit_item = dynamic_cast<UnitItem*> (_context_item);
		if (unit_item && dialog->exec() == Private::CreatePortDialog::Accepted)
		{
			Core::Port::Direction dir =
				(unit_item->treeWidget() == _opanel->list())
					? Core::Port::Output
					: Core::Port::Input;
			_unit_bay->graph()->lock();
			if (dialog->port_type() == Private::CreatePortDialog::Audio)
				new Core::AudioPort (_unit_bay, dialog->port_name().ascii(), dir);
			else
				new Core::EventPort (_unit_bay, dialog->port_name().ascii(), dir);
			_unit_bay->graph()->unlock();
		}
	}
	catch (Exception& e)
	{
		QMessageBox::warning (this, "Warning", e.what());
	}
}


void
PortsConnector::destroy_port()
{
	PortItem* item = dynamic_cast<PortItem*> (_context_item);
	// Only Unit Bay's ports can be destroyed:
	if (item && item->port()->unit() == _unit_bay)
		delete item->port();
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

	QAction* connect_action = _context_menu->addAction (Config::Icons16::connect(), "&Connect", this, SLOT (connect_selected()));
	connect_action->setEnabled (can_connect_selected());

	QAction* disconnect_action = _context_menu->addAction (Config::Icons16::disconnect(), "&Disconnect", this, SLOT (disconnect_selected()));
	disconnect_action->setEnabled (can_disconnect_selected());

	QAction* disconnect_all_action = _context_menu->addAction (Config::Icons16::disconnect(), "Disconnect all", this, SLOT (disconnect_all_from_selected()));
	disconnect_all_action->setEnabled (port_item &&
									   ((port_item->treeWidget() == _ipanel->list() && !port_item->port()->back_connections().empty()) ||
									    (port_item->treeWidget() == _opanel->list() && !port_item->port()->forward_connections().empty())));

	_context_menu->addSeparator();

	QAction* create_port_action = _context_menu->addAction (Config::Icons16::add(), "&Create port", this, SLOT (create_port()));
	create_port_action->setEnabled (dynamic_cast<UnitItem*> (item) && dynamic_cast<UnitItem*> (item)->unit() == _unit_bay);

	QAction* destroy_port_action = _context_menu->addAction (Config::Icons16::remove(), "Dest&roy port", this, SLOT (destroy_port()));
	destroy_port_action->setEnabled (dynamic_cast<PortItem*> (item) && dynamic_cast<UnitItem*> (item->parent()) && dynamic_cast<UnitItem*> (item->parent())->unit() == _unit_bay);

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
				x	port_item->port()->disconnect_from (*port_item->port()->forward_connections().begin());
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
	for (Core::Ports::iterator p = unit->inputs().begin(); p != unit->inputs().end(); ++p)
		port_registered (*p, unit);
	for (Core::Ports::iterator p = unit->outputs().begin(); p != unit->outputs().end(); ++p)
		port_registered (*p, unit);
	_unit_bay->graph()->unlock();
}


void
PortsConnector::remove_unit (Unit* unit)
{
	_unit_bay->graph()->lock();
	for (Core::Ports::iterator p = unit->inputs().begin(); p != unit->inputs().end(); ++p)
		port_unregistered (*p, unit);
	for (Core::Ports::iterator p = unit->outputs().begin(); p != unit->outputs().end(); ++p)
		port_unregistered (*p, unit);
	unit_unregistered (unit);
	_unit_bay->graph()->unlock();
}


void
PortsConnector::unit_registered (Core::Unit* core_unit)
{
	Unit* unit = dynamic_cast<Unit*> (core_unit);
	if (unit)
	{
		bool x = _unit_bay->units().find (unit) != _unit_bay->units().end();
		bool s = dynamic_cast<AudioBackend*> (unit) || dynamic_cast<EventBackend*> (unit);
		if (x || s)
		{
			if (s)
				_external_units.insert (unit);
			_ipanel->list()->insert_unit (unit);
			_opanel->list()->insert_unit (unit);
			_ipanel->filter()->read_units();
			_opanel->filter()->read_units();
		}
	}
}


void
PortsConnector::unit_unregistered (Core::Unit* core_unit)
{
	Unit* unit = dynamic_cast<Unit*> (core_unit);
	if (unit)
	{
		if (dynamic_cast<AudioBackend*> (unit) || dynamic_cast<EventBackend*> (unit))
			_external_units.erase (unit);
		_ipanel->list()->remove_unit (unit);
		_opanel->list()->remove_unit (unit);
		_ipanel->filter()->read_units();
		_opanel->filter()->read_units();
	}
}


void
PortsConnector::unit_retitled (Core::Unit* core_unit)
{
	Unit* unit = dynamic_cast<Unit*> (core_unit);
	if (unit)
	{
		_ipanel->list()->update_unit (unit);
		_opanel->list()->update_unit (unit);
		_ipanel->filter()->read_units();
		_opanel->filter()->read_units();
	}
}


void
PortsConnector::port_renamed (Core::Port* port)
{
	UnitItem* unit_item = find_unit_item (port->direction(), port->unit());
	if (unit_item)
		unit_item->update_port (port);
}


void
PortsConnector::port_connected_to (Core::Port*, Core::Port*)
{
	_connector->update();
}


void
PortsConnector::port_disconnected_from (Core::Port*, Core::Port*)
{
	_connector->update();
}


void
PortsConnector::port_registered (Core::Port* port, Core::Unit* unit)
{
	UnitItem* unit_item = find_unit_item (port->direction(), unit);
	if (unit_item)
		_ports_to_items[port] = unit_item->insert_port (port);
}


void
PortsConnector::port_unregistered (Core::Port* port, Core::Unit* unit)
{
	_ports_to_items.erase (port);
	UnitItem* unit_item = find_unit_item (port->direction(), unit);
	if (unit_item)
		unit_item->remove_port (port);
}


void
PortsConnector::port_group_renamed (Core::PortGroup*)
{
	// TODO
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
			Core::Ports const& ports = ui->unit()->inputs();
			for (Core::Ports::const_iterator port = ports.begin();  port != ports.end();  ++port)
				operate_on_ports (operation, po->port(), *port);
		}
		// Unit -> Port?
		else if (uo && pi)
		{
			Core::Ports const& ports = uo->unit()->outputs();
			for (Core::Ports::const_iterator port = ports.begin();  port != ports.end();  ++port)
				operate_on_ports (operation, *port, pi->port());
		}
		// Unit -> Unit?
		else if (uo && ui)
		{
			// Filter audio/event ports:
			std::list<Core::Port*> audio_oports, audio_iports;
			std::list<Core::Port*> event_oports, event_iports;
			for (Core::Ports::const_iterator p = uo->unit()->outputs().begin();  p != uo->unit()->outputs().end();  ++p)
			{
				if (dynamic_cast<Core::AudioPort*> (*p))
					audio_oports.push_back (*p);
				else if (dynamic_cast<Core::EventPort*> (*p))
					event_oports.push_back (*p);
			}
			for (Core::Ports::const_iterator p = ui->unit()->inputs().begin();  p != ui->unit()->inputs().end();  ++p)
			{
				if (dynamic_cast<Core::AudioPort*> (*p))
					audio_iports.push_back (*p);
				else if (dynamic_cast<Core::EventPort*> (*p))
					event_iports.push_back (*p);
			}
			// Audio:
			{
				std::list<Core::Port*>::const_iterator o = audio_oports.begin();
				std::list<Core::Port*>::const_iterator i = audio_iports.begin();
				for (; o != audio_oports.end() && i != audio_iports.end();  ++o, ++i)
					operate_on_ports (operation, *o, *i);
			}
			// Event:
			{
				std::list<Core::Port*>::const_iterator o = event_oports.begin();
				std::list<Core::Port*>::const_iterator i = event_iports.begin();
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
			|| (po && ui && ui->count_inputs_if (std::not1 (Core::connected_from (po->port()))) > 0)
			|| (uo && pi && uo->count_outputs_if (std::not1 (Core::connected_to (pi->port()))) > 0)
			|| (uo && ui); // TODO: && istnieją takie, które nie są połączone, a były by po połączeniu:
	else
		return (po && pi && po->port()->connected_to (pi->port()))
			|| (po && ui && ui->count_inputs_if (Core::connected_from (po->port())) > 0)
			|| (uo && pi && uo->count_outputs_if (Core::connected_to (pi->port())) > 0)
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
			Core::Ports const& iports = oportitem->port()->forward_connections();
			_unit_bay->graph()->lock();
			for (Core::Ports::iterator p = iports.begin(); p != iports.end(); ++p)
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
			Core::Ports const& oports = iportitem->port()->back_connections();
			for (Core::Ports::iterator p = oports.begin(); p != oports.end(); ++p)
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
PortsConnector::find_unit_item (Core::Port::Direction direction, Core::Unit* core_unit) const
{
	Unit* unit = dynamic_cast<Unit*> (core_unit);
	if (unit)
	{
		PortsList::UnitsToItemsMap const* units_list = 0;
		switch (direction)
		{
			case Core::Port::Input:		units_list = _ipanel->list()->units(); break;
			case Core::Port::Output:	units_list = _opanel->list()->units(); break;
		}
		PortsList::UnitsToItemsMap::const_iterator unit_item = units_list->find (unit);
		if (unit_item != units_list->end())
			return unit_item->second;
	}
	return 0;
}


Private::PortItem*
PortsConnector::find_port_item (Core::Port* port) const
{
	PortsToItemsMap::const_iterator p = _ports_to_items.find (port);
	if (p != _ports_to_items.end())
		return p->second;
	return 0;
}

} // namespace Haruhi

