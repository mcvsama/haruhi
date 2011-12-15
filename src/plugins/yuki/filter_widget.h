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

#ifndef HARUHI__PLUGINS__YUKI__FILTER_WIDGET_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__FILTER_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>

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
	params_updated();

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
	Params::Filter*					_params;
	FilterImpulseResponse			_impulse_response;
	Part*							_part;

	// Knobs:
	Haruhi::Knob*					_knob_frequency;
	Haruhi::Knob*					_knob_resonance;
	Haruhi::Knob*					_knob_gain;
	Haruhi::Knob*					_knob_attenuation;

	QWidget*						_panel;
	QCheckBox*						_enabled_widget;
	Haruhi::FrequencyResponsePlot*	_response_plot;
	QComboBox*						_filter_type;
	QSpinBox*						_stages;
	QCheckBox*						_limiter_enabled;
};

} // namespace Yuki

#endif

