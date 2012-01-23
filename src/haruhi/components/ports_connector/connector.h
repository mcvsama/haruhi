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

#ifndef HARUHI__COMPONENTS__PORTS_CONNECTOR__CONNECTOR_H__INCLUDED
#define HARUHI__COMPONENTS__PORTS_CONNECTOR__CONNECTOR_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

class PortsConnector;

namespace PortsConnectorPrivate {

/**
 * This widget draws lines between two QTreeWidgets in PortsConnector.
 */
class Connector: public QWidget
{
  public:
	typedef std::list<QTreeWidgetItem*> QTreeWidgetItemsList;

  public:
	Connector (PortsConnector* ports_connector, QWidget* parent = 0);

	virtual ~Connector() { }

  protected:
	void
	paintEvent (QPaintEvent*);

	void
	resizeEvent (QResizeEvent*);

  private:
	void
	draw_connections();

	void
	draw_line (QPainter& p, int x1, int y1, int x2, int y2, int h1, int h2);

	bool
	item_visible (QTreeWidgetItem*);

	static int
	item_y (QTreeWidgetItem*);

	/**
	 * Returns all descendants of given QTreeWidget as a list.
	 */
	static QTreeWidgetItemsList
	get_all_items_from (QTreeWidget*);

	/**
	 * Returns all descendants of given QTreeWidgetItem as a list.
	 */
	static QTreeWidgetItemsList
	get_all_items_from (QTreeWidgetItem*);

  private:
	PortsConnector*	_ports_connector;
	QPixmap			_double_buffer;
};

} // namespace PortsConnectorPrivate

} // namespace Haruhi

#endif

