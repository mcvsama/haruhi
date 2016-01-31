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

#ifndef HARUHI__PLUGINS__YUKI__PART_MANAGER_WIDGET_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_MANAGER_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QSpinBox>
#include <QTabBar>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/signal.h>
#include <haruhi/widgets/knob.h>


namespace Yuki {

using Haruhi::Unique;

class Plugin;
class Part;
class PartManager;

class PartManagerWidget:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

	class Placeholder: public QWidget
	{
	  public:
		Placeholder (QWidget* parent);
	};

	/**
	 * Make QTabWidget::tabBar() method publicly accessible.
	 */
	class TabWidget: public QTabWidget
	{
	  public:
		TabWidget (QWidget* parent);

		QTabBar*
		tabBar() const;
	};

  public:
	PartManagerWidget (QWidget* parent, PartManager*);

	Plugin*
	plugin() const;

	/**
	 * Set UnitBay object from the Plugin.
	 */
	void
	unit_bay_assigned();

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

	/**
	 * Update tab ID for given Part.
	 */
	void
	update_part (Part*);

  private slots:
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

	/**
	 * Called when tab is moved in tabs.
	 */
	void
	tab_moved (int old_position, int new_position);

	/**
	 * Update params from widget states.
	 */
	void
	widgets_to_params();

  private:
	/**
	 * Updates widgets' states from params.
	 * \entry	UI thread only
	 */
	void
	params_to_widgets();

	/**
	 * Call params_to_widgets from UI thread later.
	 * \entry	any thread
	 */
	void
	post_params_to_widgets();

	/**
	 * Update dependent widgets.
	 */
	void
	update_widgets();

  private:
	PartManager*			_part_manager;
	bool					_stop_widgets_to_params;
	bool					_stop_params_to_widgets;

	Unique<TabWidget>		_tabs;
	Unique<QPushButton>		_add_part_button;
	Unique<QPushButton>		_remove_part_button;
	Unique<QWidget>			_placeholder;
	Unique<QSpinBox>		_polyphony;
	Unique<QSpinBox>		_oversampling;

	Unique<Haruhi::Knob>	_knob_volume;
	Unique<Haruhi::Knob>	_knob_panorama;
	Unique<Haruhi::Knob>	_knob_detune;
	Unique<Haruhi::Knob>	_knob_stereo_width;
};


inline PartManagerWidget::TabWidget::TabWidget (QWidget* parent):
	QTabWidget (parent)
{ }


inline QTabBar*
PartManagerWidget::TabWidget::tabBar() const
{
	return QTabWidget::tabBar();
}

} // namespace Yuki

#endif

