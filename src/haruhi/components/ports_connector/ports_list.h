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

#ifndef HARUHI__COMPONENTS__PORTS_CONNECTOR__PORTS_LIST_H__INCLUDED
#define HARUHI__COMPONENTS__PORTS_CONNECTOR__PORTS_LIST_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QTimer>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "ports_connector.h"
#include "unit_item.h"


namespace Haruhi {

class PortsConnector;

namespace PortsConnectorPrivate {

class Panel;

class PortsList: public QTreeWidget
{
	Q_OBJECT

	friend class UnitItem;

  public:
	typedef std::map<Unit*, UnitItem*> UnitsToItemsMap;

  public:
	PortsList (Port::Direction type, PortsConnectorPrivate::Panel* panel, PortsConnector* ports_connector, QWidget* parent);

	~PortsList();

	UnitsToItemsMap const*
	units() const;

	PortsConnector*
	ports_connector();

	QTreeWidgetItem*
	selected_item() const;

  public slots:
	void
	read_units();

	void
	insert_unit (Unit*);

	void
	remove_unit (Unit*);

	void
	update_unit (Unit*);

	/**
	 * Sets filter to show only given unit.
	 * 0 means no filtering.
	 */
	void
	set_filter (Unit* unit);

	bool
	unit_exist (Unit*) const;

	void
	auto_open_selected();

  protected:
	/**
	 * Drag and drop methods.
	 */

	void
	dragEnterEvent (QDragEnterEvent*) override;

	void
	dragMoveEvent (QDragMoveEvent*) override;

	void
	dragLeaveEvent (QDragLeaveEvent*) override;

	void
	dropEvent (QDropEvent*) override;

	void
	mousePressEvent (QMouseEvent*) override;

	void
	mouseMoveEvent (QMouseEvent*) override;

  private:
	QTreeWidgetItem*
	drag_drop_item (QPoint const&);

	void
	update_filter();

	void
	sort();

  private slots:
	void
	context_menu (const QPoint&);

  signals:
	void
	context_menu (QTreeWidgetItem*, QPoint const&);

  private:
	Port::Direction					_type;
	PortsConnectorPrivate::Panel*	_panel;
	PortsConnector*					_ports_connector;
	QTreeWidgetItem*				_drag_drop_item = nullptr;
	UnitsToItemsMap					_units;
	Unique<QTimer>					_auto_open_timer;
	Unit*							_filter			= nullptr;	// If not 0, show only items belonging to _filter.
	QPoint							_drag_pos;					// Drag init point.
	QTreeWidgetItem*				_dragged_item	= nullptr;
};


inline PortsList::UnitsToItemsMap const*
PortsList::units() const
{
	return &_units;
}


inline PortsConnector*
PortsList::ports_connector()
{
	return _ports_connector;
}


inline void
PortsList::sort()
{
	sortByColumn (0, Qt::AscendingOrder);
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

#endif

