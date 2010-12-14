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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__PANEL_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__PANEL_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QPushButton>

// Haruhi:
#include <haruhi/components/event_backend/backend.h>
#include <haruhi/utility/signal.h>

// Local:
#include "ports_list_view.h"


namespace Haruhi {

namespace DevicesManager {

class DeviceDialog;
class DeviceItem;
class ControllerDialog;
class ControllerItem;

class Panel:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

	typedef std::set<ControllerItem*> LearningItems;

  public:
	Panel (QWidget* parent);

	~Panel();

	/**
	 * Callback for EventBackend's on_event.
	 * Passed to controller items for learning.
	 */
	void
	on_event (EventBackendImpl::Transport::MidiEvent const& event);

  public slots:
	/**
	 * Creates new unnamed/unconfigured device
	 * and inserts it into the tree.
	 */
	void
	create_device();

	/**
	 * Adds Device as new template.
	 */
	void
	add_device (DeviceItem* device_item);

	/**
	 * Creates new unnamed/unconfigured controller
	 * and inserts it into the subtree of currently selected device.
	 * If no device is selected, it does nothing.
	 */
	void
	create_controller();

  private slots:
	void
	update_widgets();

	void
	selection_changed();

	void
	configure_item (DeviceItem* item);

	void
	configure_item (ControllerItem* item);

	void
	configure_selected_item();

	void
	learn_from_midi();

	void
	destroy_selected_item();

	void
	context_menu_for_items (QPoint const&);

	/**
	 * Loads settings from DevicesManagerSettings object.
	 */
	void
	load_settings();

	/**
	 * Saves devices to DevicesManagerSettings object.
	 */
	void
	save_settings();

  private:
	QStackedWidget*		_stack;
	QPushButton*		_create_device_button;
	QPushButton*		_create_controller_button;
	QPushButton*		_destroy_input_button;
	PortsListView*		_tree;
	DeviceDialog*		_device_dialog;
	ControllerDialog*	_controller_dialog;
	LearningItems		_learning_items;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

