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

// Qt:
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include <QHeaderView>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "connector.h"
#include "ports_connector.h"


namespace Haruhi {

namespace PortsConnectorPrivate {

using namespace ScreenLiterals;


Connector::Connector (PortsConnector* ports_connector, QWidget* parent):
	QWidget (parent),
	_ports_connector (ports_connector)
{
	setAttribute (Qt::WA_NoBackground);
	setMinimumWidth (100);
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}


void
Connector::paintEvent (QPaintEvent*)
{
	draw_connections();
}


void
Connector::resizeEvent (QResizeEvent*)
{
	QWidget::repaint();
}


void
Connector::draw_connections()
{
	_double_buffer = QPixmap (size());
	QPainter painter (&_double_buffer);
	painter.setRenderHint (QPainter::Antialiasing, true);

	int color = 0;
	int rgb[] = { 0x44, 0x88, 0xcc };

	int x1 = 0;
	int x2 = this->width();
	int h1 = _ports_connector->_opanel->list()->header()->height();
	int h2 = _ports_connector->_ipanel->list()->header()->height();

	painter.fillRect (rect(), palette().color (QPalette::Window));

	for (QTreeWidgetItem* oitem: Connector::get_all_items_from (_ports_connector->_opanel->list()))
	{
		if (!item_visible (oitem))
			continue;

		PortItem* oaudio_item = dynamic_cast<PortItem*> (oitem);
		PortItem* oevent_item = dynamic_cast<PortItem*> (oitem);

		// Rotate color:
		color += 1;
		painter.setPen (QPen (QColor (rgb[color % 3], rgb[(color / 3) % 3], rgb[(color / 9) % 3]), 0.15_screen_mm));

		for (QTreeWidgetItem* iitem: Connector::get_all_items_from (_ports_connector->_ipanel->list()))
		{
			if (!item_visible (iitem))
				continue;

			PortItem* iaudio_item = dynamic_cast<PortItem*> (iitem);
			PortItem* ievent_item = dynamic_cast<PortItem*> (iitem);

			if (oaudio_item && iaudio_item && oaudio_item->port()->connected_to (iaudio_item->port()))
			{
				float y1 = Connector::item_y (oaudio_item) + 0.5;
				float y2 = Connector::item_y (iaudio_item) + 0.5;
				draw_line (painter, x1, y1, x2, y2, h1, h2);
			}

			if (oevent_item && ievent_item && oevent_item->port()->connected_to (ievent_item->port()))
			{
				float y1 = Connector::item_y (oevent_item) + 0.5;
				float y2 = Connector::item_y (ievent_item) + 0.5;
				draw_line (painter, x1, y1, x2, y2, h1, h2);
			}
		}
	}

	QPainter (this).drawPixmap (0, 0, _double_buffer);
}


void
Connector::draw_line (QPainter& painter, int x1, int y1, int x2, int y2, int h1, int h2)
{
	// Account for headers and frame and relative position:
	y1 += h1 + 3 + _ports_connector->_opanel->list()->pos().y();
	y2 += h2 + 3 + _ports_connector->_ipanel->list()->pos().y();

    // Invisible output ports don't get a connecting dot:
	if (y1 > h1)
		painter.drawLine (x1, y1, x1 + 4, y1);

	// Line:
	int cp = static_cast<int> (static_cast<double> (x2 - x1 - 8) * 0.4);
	QPolygon spline;
	spline << QPoint (x1 + 4, y1);
	spline << QPoint (x1 + 4 + cp, y1);
	spline << QPoint (x2 - 4 - cp, y2);
	spline << QPoint (x2 - 4, y2);

	QPainterPath path;
	path.moveTo (spline[0]);
	path.cubicTo (spline[1], spline[2], spline[3]);
	painter.strokePath (path, painter.pen());

    // Invisible input ports don't get a connecting dot:
	if (y2 > h2)
		painter.drawLine (x2 - 4, y2, x2, y2);
}


bool
Connector::item_visible (QTreeWidgetItem* item)
{
	bool shown = true;
	for (; shown && item; item = item->parent())
		shown = shown && !item->isHidden();
	return shown;
}


int
Connector::item_y (QTreeWidgetItem* item)
{
	QTreeWidget* list_view = item->treeWidget();
	// f: first closed item counting from root item:
	QTreeWidgetItem* i = item;
	QTreeWidgetItem* f = item;
	do {
		if (!i->isExpanded())
			f = i;
		i = i->parent();
	} while (i != 0);

	QRect rect = list_view->visualItemRect (f);
	return rect.top() + rect.height() / 2;
}


Connector::QTreeWidgetItemsList
Connector::get_all_items_from (QTreeWidget* parent)
{
	QTreeWidgetItemsList result;
	for (int i = 0; i < parent->invisibleRootItem()->childCount(); ++i)
	{
		QTreeWidgetItem* item = parent->invisibleRootItem()->child (i);
		result.push_back (item);
		QTreeWidgetItemsList aux = Connector::get_all_items_from (item);
		std::copy (aux.begin(), aux.end(), std::back_inserter (result));
	}
	return result;
}


Connector::QTreeWidgetItemsList
Connector::get_all_items_from (QTreeWidgetItem* parent)
{
	QTreeWidgetItemsList result;
	for (int i = 0; i < parent->childCount(); ++i)
	{
		QTreeWidgetItem* item = parent->child (i);
		result.push_back (item);
		QTreeWidgetItemsList aux = Connector::get_all_items_from (item);
		std::copy (aux.begin(), aux.end(), std::back_inserter (result));
	}
	return result;
}

} // namespace PortsConnectorPrivate

} // namespace Haruhi

