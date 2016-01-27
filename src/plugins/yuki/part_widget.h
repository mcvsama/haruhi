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

#ifndef HARUHI__PLUGINS__YUKI__PART_WIDGET_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>
#include <vector>

// Qt:
#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QStackedWidget>
#include <QTabWidget>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/widgets/wave_plot.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/utility/signal.h>

// Local:
#include "part.h"


namespace Yuki {

namespace DSP = Haruhi::DSP;
using Haruhi::Unique;

class Part;
class PartManagerWidget;
class PartOscillatorWidget;
class PartModulatorWidget;
class PartHarmonicsWidget;

class PartWidget:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

  public:
	PartWidget (PartManagerWidget*, Part*);

	/**
	 * Link to associated Part object.
	 */
	Part*
	part() const;

  private slots:
	/**
	 * Called when wave-related widgets are manipulated.
	 * \entry	UI thread
	 */
	void
	widgets_to_wave_params();

	/**
	 * Called when oscillator-related widgets are manipulated.
	 */
	void
	widgets_to_oscillator_params();

	/**
	 * Update widgets deps (enable/disable, etc.)
	 */
	void
	update_widgets();

	/**
	 * Update phase marker on wave plots.
	 * \entry	UI thread
	 */
	void
	update_phase_marker();

  public:
	/**
	 * Updates widgets' states from params.
	 * \entry   UI thread only
	 */
	void
	params_to_widgets();

	/**
	 * Connected to part's waves_updated signal.
	 * \entry	UI thread
	 */
	void
	update_wave_plots();

	/**
	 * Call update_wave_plots() from UI thread later.
	 * \entry	any thread
	 */
	void
	post_update_wave_plots();

	/**
	 * Call params_to_widgets from UI thread later.
	 * \entry	any thread
	 */
	void
	post_params_to_widgets();

  private:
	PartManagerWidget*		_part_manager_widget;
	Part*					_part;
	Unique<DSP::Wave>		_cached_final_wave;
	bool					_stop_widgets_to_params;
	bool					_stop_params_to_widgets;

	// Other:
	QTabWidget*				_tabs;
	PartOscillatorWidget*	_oscillator_panel;
	PartModulatorWidget*	_modulator_panel;
	PartHarmonicsWidget*	_harmonics_panel;
	QCheckBox*				_part_enabled;
};


inline Part*
PartWidget::part() const
{
	return _part;
}

} // namespace Yuki

#endif

