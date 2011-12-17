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
#include <QtGui/QSpinBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/signal.h>
#include <haruhi/widgets/knob.h>


namespace Yuki {

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
	 * Update params from widget states.
	 */
	void
	widgets_to_params();

  private:
	/**
	 * Update dependent widgets.
	 */
	void
	update_widgets();

  private:
	PartManager*	_part_manager;
	QTabWidget*		_tabs;
	QPushButton*	_add_part_button;
	QPushButton*	_remove_part_button;
	QWidget*		_placeholder;
	QSpinBox*		_polyphony;

	Haruhi::Knob*	_knob_volume;
	Haruhi::Knob*	_knob_panorama;
	Haruhi::Knob*	_knob_detune;
	Haruhi::Knob*	_knob_stereo_width;
};

} // namespace Yuki

#endif

