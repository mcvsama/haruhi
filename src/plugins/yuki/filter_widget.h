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

#ifndef HARUHI__PLUGINS__YUKI__FILTER_WIDGET_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__FILTER_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/widgets/frequency_response_plot.h>
#include <haruhi/utility/signal.h>

// Local:
#include "params.h"
#include "filter_ir.h"


namespace Yuki {

namespace DSP = Haruhi::DSP;
using Haruhi::Unique;

class Part;

class FilterWidget:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

  public:
	FilterWidget (QWidget* parent, unsigned int filter_no, Params::Filter*, Part*);

	~FilterWidget();

  private slots:
	/**
	 * Update params from widgets.
	 * Call it when widgets are changed.
	 */
	void
	widgets_to_params();

	/**
	 * Update widget states, replot transmittance, etc.
	 */
	void
	update_widgets();

	/**
	 * Update impulse response from params.
	 */
	void
	update_impulse_response();

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

  private:
	Params::Filter*							_params;
	FilterImpulseResponse					_impulse_response;
	Part*									_part;
	bool									_stop_widgets_to_params;
	bool									_stop_params_to_widgets;

	// In this order:
	Unique<QWidget>							_panel;
	Unique<QCheckBox>						_enabled_widget;
	Unique<Haruhi::FrequencyResponsePlot>	_response_plot;
	Unique<QComboBox>						_filter_type;
	Unique<QSpinBox>						_stages;
	Unique<QCheckBox>						_limiter_enabled;

	// Knobs:
	Unique<Haruhi::Knob>					_knob_frequency;
	Unique<Haruhi::Knob>					_knob_resonance;
	Unique<Haruhi::Knob>					_knob_gain;
	Unique<Haruhi::Knob>					_knob_attenuation;
};

} // namespace Yuki

#endif

