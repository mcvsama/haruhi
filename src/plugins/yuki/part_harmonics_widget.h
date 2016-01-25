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

#ifndef HARUHI__PLUGINS__YUKI__PART_HARMONICS_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_HARMONICS_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>
#include <vector>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QSlider>
#include <QtGui/QPushButton>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>

// Local:
#include "params.h"


namespace Yuki {

class Part;
class PartWidget;


/**
 * Slider that implements slot reset() which
 * sets slider value to 0.
 */
class Slider: public QSlider
{
	Q_OBJECT

  public:
	Slider (int min_value, int max_value, int page_step, int value, Qt::Orientation orientation, QWidget* parent);

  public slots:
	/**
	 * Set slider value to 0.
	 */
	void
	reset();
};


class PartHarmonicsWidget: public QWidget
{
	Q_OBJECT

	typedef std::vector<QSlider*>		Sliders;
	typedef std::vector<QPushButton*>	Buttons;

  public:
	PartHarmonicsWidget (QWidget* parent, PartWidget* part_widget, Part* part);

  public slots:
	/**
	 * Called when wave-related widgets are manipulated.
	 * \entry	UI thread
	 */
	void
	widgets_to_wave_params();

	/**
	 * Update widgets deps (enable/disable, etc.)
	 */
	void
	update_widgets();

	/**
	 * Updates widgets' states from params.
	 * \entry   UI thread only
	 */
	void
	params_to_widgets();

  private slots:
	/**
	 * Reset all harmonics to default values.
	 */
	void
	reset_all_harmonics();

  private:
	/**
	 * Highlights selected button.
	 */
	void
	set_button_highlighted (QPushButton* button, bool highlight);

  private:
	PartWidget*	_part_widget;
	Part*		_part;

	Sliders		_harmonics_sliders;
	Buttons		_harmonics_resets;
	Sliders		_harmonic_phases_sliders;
	Buttons		_harmonic_phases_resets;

	QColor		_std_button_bg;
	QColor		_std_button_fg;
};

} // namespace Yuki

#endif

