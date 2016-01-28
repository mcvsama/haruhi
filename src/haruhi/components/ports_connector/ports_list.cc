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

// Standard:
#include <cstddef>
#include <algorithm>
#include <iterator>
#include <set>

// Qt:
#include <QApplication>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDrag>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "ports_list.h"
#include "ports_connector.h"
#include "panel.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

PortsList::PortsList (Port::Direction type, PortsConnector::Panel* panel, PortsConnector* ports_connector, QWidget* parent):
	QTreeWidget (parent),
	_type (type),
	_panel (panel),
	_ports_connector (ports_connector)
{
	header()->setSectionsClickable (false);
	header()->setSectionResizeMode (QHeaderView::Stretch);
	sortByColumn (0, Qt::AscendingOrder);
	setMinimumWidth (160);
	setSortingEnabled (true);
	setSelectionMode (QTreeWidget::SingleSelection);
	setRootIsDecorated (true);
	setAllColumnsShowFocus (true);
	setAcceptDrops (true);
	setAutoScroll (true);
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
	setContextMenuPolicy (Qt::CustomContextMenu);
	setIconSize (Resources::Icons16::haruhi().size());

	switch (_type)
	{
		case Port::Input:	setHeaderLabel ("Input ports"); break;
		case Port::Output:	setHeaderLabel ("Output ports"); break;
	}

	_auto_open_timer = std::make_unique<QTimer> (this);
	_auto_open_timer->setSingleShot (true);
	QObject::connect (_auto_open_timer.get(), SIGNAL (timeout()), this, SLOT (auto_open_selected()));
	QObject::connect (this, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu (const QPoint&)));
}


PortsList::~PortsList()
{
	_auto_open_timer->stop();
}


QTreeWidgetItem*
PortsList::selected_item() const
{
	auto list = selectedItems();
	return list.empty() ? 0 : list.front();
}


void
PortsList::read_units()
{
	typedef Graph::Units Units;

	Graph* graph = _ports_connector->unit_bay()->graph();

	Units g_units; // Graph units
	Units l_units; // List units
	for (auto u: graph->units())
		g_units.insert (u);
	for (auto u: _units)
		l_units.insert (u.first);

	Units added;
	Units removed;
	Units rest;
	std::set_difference (g_units.begin(), g_units.end(), l_units.begin(), l_units.end(), std::inserter (added, added.end()));
	std::set_difference (l_units.begin(), l_units.end(), g_units.begin(), g_units.end(), std::inserter (removed, removed.end()));
	std::set_intersection (g_units.begin(), g_units.end(), l_units.begin(), l_units.end(), std::inserter (rest, rest.end()));

	// Removing items for deleted units first is safest.
	for (Unit* u: removed)
		remove_unit (u);
	for (Unit* u: added)
		insert_unit (u);
	for (Unit* u: rest)
		update_unit (u);

	sort();
}


void
PortsList::insert_unit (Unit* unit)
{
	if (!unit_exist (unit))
	{
		auto unit_item = new UnitItem (_type, unit, this, QString::fromStdString (unit->title()));
		invisibleRootItem()->addChild (unit_item);
		unit_item->setExpanded (true);
		_units[unit] = unit_item;
	}
}


void
PortsList::remove_unit (Unit* unit)
{
	if (unit_exist (unit))
	{
		auto li = _units[unit];
		if (li->parent())
			li->parent()->removeChild (li);
		else
			li->treeWidget()->invisibleRootItem()->removeChild (li);
		delete li;
		_units.erase (unit);
	}
}


void
PortsList::update_unit (Unit* unit)
{
	_units[unit]->update();
	_units[unit]->read_ports();
}


void
PortsList::set_filter (Unit* unit)
{
	_filter = unit;
	update_filter();
}


bool
PortsList::unit_exist (Unit* unit) const
{
	return _units.find (unit) != _units.end();
}


void
PortsList::auto_open_selected()
{
	auto item = selected_item();
	if (item)
		item->setExpanded (true);
}


void
PortsList::dragEnterEvent (QDragEnterEvent* event)
{
	PortsList* source;
	if (event->source() && (source = dynamic_cast<PortsList*> (event->source())) &&
		source != this && event->mimeData()->hasText() && drag_drop_item (event->pos()))
	{
		event->accept();
	}
	else
		event->ignore();
}


void
PortsList::dragMoveEvent (QDragMoveEvent* event)
{
	PortsList* source;
	if (event->source() && (source = dynamic_cast<PortsList*> (event->source())) && source != this && event->mimeData()->hasText())
	{
		auto oitem = source->selectedItems().empty() ? 0 : source->selectedItems().front();
		auto iitem = drag_drop_item (event->pos());

		if (oitem->treeWidget() != _ports_connector->_opanel->list())
			std::swap (oitem, iitem);

		if (oitem && iitem && _ports_connector->can_connect (oitem, iitem))
			event->accept();
		else
			event->ignore();
	}
	else
		event->ignore();
}


void
PortsList::dragLeaveEvent (QDragLeaveEvent*)
{
	_drag_drop_item = nullptr;
}


void
PortsList::dropEvent (QDropEvent* event)
{
	if (event->source() != this)
	{
		if (event->mimeData()->hasText() && drag_drop_item (event->pos()))
			_ports_connector->connect_selected();
	}
	event->acceptProposedAction();
	dragLeaveEvent (0);
}


void
PortsList::mousePressEvent (QMouseEvent* mouse_event)
{
	QTreeWidget::mousePressEvent (mouse_event);

	if (mouse_event->button() == Qt::LeftButton)
	{
		_drag_pos = mouse_event->pos();
		_dragged_item = itemAt (_drag_pos);
	}
}


void
PortsList::mouseMoveEvent (QMouseEvent* mouse_event)
{
	QTreeWidget::mouseMoveEvent (mouse_event);

	if ((mouse_event->buttons() & Qt::LeftButton) && _dragged_item && ((mouse_event->pos() - _drag_pos).manhattanLength() >= QApplication::startDragDistance()))
	{
		auto mime_data = new QMimeData();
		mime_data->setText (_dragged_item->text (0));
		auto drag = new QDrag (this);
		drag->setMimeData (mime_data);
		drag->setPixmap (_dragged_item->icon (0).pixmap (16));
		drag->setHotSpot (QPoint (-4, -12));
		drag->start (Qt::LinkAction);
		// We've dragged and maybe dropped it by now...
		_dragged_item = nullptr;
	}
}


QTreeWidgetItem*
PortsList::drag_drop_item (QPoint const& epos)
{
	auto item = itemAt (epos);
	if (item)
	{
		if (item != _drag_drop_item)
		{
			clearSelection();
			item->setSelected (true);
			_drag_drop_item = item;
			_auto_open_timer->start (750);
		}
	}
	else
	{
		_drag_drop_item = nullptr;
		_auto_open_timer->stop();
	}
	return item;
}


void
PortsList::update_filter()
{
	for (int i = 0; i < invisibleRootItem()->childCount(); ++i)
	{
		auto unit_item = dynamic_cast<UnitItem*> (invisibleRootItem()->child (i));
		if (unit_item)
			unit_item->set_filtered_out (!(_filter == 0 || unit_item->unit() == _filter));
	}
}


void
PortsList::context_menu (QPoint const& pos)
{
	auto item = itemAt (pos);
	if (!item)
		return;
	emit context_menu (item, QCursor::pos());
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

