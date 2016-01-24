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
#include <QtGui/QDialog>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>
#include <QtGui/QStackedWidget>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/widgets/wave_plot.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/utility/signal.h>

// Local:
#include "params.h"


namespace Yuki {

namespace DSP = Haruhi::DSP;

class Part;
class PartWidget;


class PartHarmonicsWidget: public QWidget
{
	Q_OBJECT

	friend class PartWidget;

	typedef std::vector<QSlider*>		Sliders;
	typedef std::vector<QPushButton*>	Buttons;

  public:
	PartHarmonicsWidget (QWidget* parent, PartWidget* part_widget, Part* part);

  private slots:
	/**
	 * Called when wave-related widgets are manipulated.
	 * \entry	UI thread
	 */
	void
	widgets_to_wave_params();

	/**
	 * Reset all harmonics to default values.
	 */
	void
	reset_all_harmonics();

	/**
	 * Update widgets deps (enable/disable, etc.)
	 */
	void
	update_widgets();

  private:
	/**
	 * Updates widgets' states from params.
	 * \entry   UI thread only
	 */
	void
	params_to_widgets();

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


inline void
PartHarmonicsWidget::set_button_highlighted (QPushButton* button, bool highlight)
{
	QPalette p = button->palette();
	p.setColor (QPalette::Button, highlight ? QColor (0x00, 0xff, 0x00) : _std_button_bg);
	p.setColor (QPalette::ButtonText, highlight ? QColor (0x00, 0x00, 0x00) : _std_button_fg);
	button->setPalette (p);
}


inline
Slider::Slider (int min_value, int max_value, int page_step, int value, Qt::Orientation orientation, QWidget* parent):
	QSlider (orientation, parent)
{
	setMinimum (min_value);
	setMaximum (max_value);
	setPageStep (page_step);
	setValue (value);
}


inline void
Slider::reset()
{
	setValue (0);
}

} // namespace Yuki

#endif

