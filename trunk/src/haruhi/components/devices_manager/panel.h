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
#include <haruhi/lib/midi.h>
#include <haruhi/utility/signal.h>


namespace Haruhi {

namespace DevicesManager {

class Tree;
class Settings;
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
	/**
	 * \param	parent Parent widget.
	 * \param	settings Settings object used as a model.
	 */
	Panel (QWidget* parent, Settings* settings);

	~Panel();

	/**
	 * Callback for EventBackend's on_event.
	 * Passed to controller items for learning.
	 */
	void
	on_event (MIDI::Event const& event);

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
	context_menu_for_items (QPoint const&);

	void
	save_settings();

  private:
	Tree*				_tree;
	Settings*			_settings;
	QStackedWidget*		_stack;
	QPushButton*		_create_device_button;
	QPushButton*		_create_controller_button;
	QPushButton*		_destroy_input_button;
	DeviceDialog*		_device_dialog;
	ControllerDialog*	_controller_dialog;
	LearningItems		_learning_items;
};

} // namespace DevicesManager

} // namespace Haruhi

#endif

