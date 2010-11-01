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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__PORTS_LIST_VIEW_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__PORTS_LIST_VIEW_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QEvent>
#include <QtGui/QTreeWidget>

// Haruhi:
#include <haruhi/utility/saveable_state.h>
#include <haruhi/components/devices_manager/ports_list_view.h>


namespace Haruhi {

class EventBackend;

namespace EventBackendPrivate {

using DevicesManager::DeviceItem;

class PortsListView: public DevicesManager::PortsListView
{
  public:
	/**
	 * Due to bugs in Qt's MOC (Meta Object Compiler) we cannot create
	 * PortItem that is also QObject and Q_OBJECT in non-default namespace,
	 * therefore we can't send events to PortItems directly. We must
	 * do it in PortsListView.
	 */
	class LearnedParams: public QEvent
	{
	  public:
		LearnedParams (QTreeWidgetItem* item):
			QEvent (QEvent::User),
			item (item)
		{ }

	  public:
		QTreeWidgetItem* item;
	};

  public:
	PortsListView (QWidget* parent, EventBackend*);

	EventBackend*
	backend() const { return _backend; }

	/**
	 * Allocates DeviceItem that will be used
	 * as child for this PortsList.
	 */
	DeviceItem*
	create_device_item (QString const& name);

  protected:
	void
	customEvent (QEvent*);

  private:
	EventBackend* _backend;
};

} // namespace EventBackendPrivate

} // namespace Haruhi

#endif

