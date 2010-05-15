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


namespace Haruhi {

class EventBackend;

namespace EventBackendPrivate {

class PortItem;

class PortsListView:
	public QTreeWidget,
	public SaveableState
{
  public:
	/**
	 * Due to bugs in Qt's MOC (Meta Object Compiler) we cannot create
	 * PortItem that is also QObject and Q_OBJECT in non-default namespace,
	 * therefore we can't send events to PortItems directly. We must
	 * do it in PortsListView.
	 */
	class ResetIconEvent: public QEvent
	{
	  public:
		ResetIconEvent (PortItem* port_item):
			QEvent (QEvent::User),
			port_item (port_item)
		{ }

	  public:
		PortItem* port_item;
	};

  public:
	PortsListView (QWidget* parent, EventBackend*, const char* header_title);

	EventBackend*
	backend() const { return _backend; }

	QTreeWidgetItem*
	selected_item() const;

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  protected:
	void
	customEvent (QEvent*);

  private:
	EventBackend* _backend;
};

} // namespace EventBackendPrivate

} // namespace Haruhi

#endif

