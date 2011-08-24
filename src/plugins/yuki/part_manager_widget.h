/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__PLUGINS__YUKI__PART_MANAGER_WIDGET_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_MANAGER_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>

// Haruhi:
#include <haruhi/config/all.h>


namespace Yuki {

class Plugin;
class Part;
class PartManager;

class PartManagerWidget: public QWidget
{
	Q_OBJECT

  public:
	PartManagerWidget (QWidget* parent, PartManager*);

	Plugin*
	plugin() const;

	/**
	 * Create UI widget for given Part object and add it to tabs.
	 */
	void
	add_part (Part*);

	/**
	 * Remove and delete UI widget for given Part object.
	 */
	void
	remove_part (Part*);

  private slots:
	/**
	 * Show main control panel.
	 */
	void
	show_main();

	/**
	 * Show part tabs.
	 */
	void
	show_parts();

	/**
	 * Add new part.
	 * Callback for 'Add part' button.
	 */
	void
	add_part();

	/**
	 * Remove currently selected part.
	 * Callback for 'Remove part' button.
	 */
	void
	remove_current_part();

  private:
	/**
	 * Update dependent widgets.
	 */
	void
	update_widgets();

  private:
	PartManager*	_part_manager;
	QStackedWidget*	_stack;
	QWidget*		_main;
	QTabWidget*		_tabs;
	QPushButton*	_show_main_button;
	QPushButton*	_show_tabs_button;
	QPushButton*	_add_part_button;
	QPushButton*	_remove_part_button;
	bool			_prevent_recursion;
};

} // namespace Yuki

#endif

