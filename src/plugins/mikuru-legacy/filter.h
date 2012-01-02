/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__PLUGINS__MIKURU__FILTER_H__INCLUDED
#define HARUHI__PLUGINS__MIKURU__FILTER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/dsp/wave.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/graph/port_group.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/widgets/frequency_response_plot.h>
#include <haruhi/utility/signal.h>

// Local:
#include "rbj_impulse_response.h"
#include "widgets.h"
#include "params.h"
#include "event_dispatcher.h"


class Mikuru;

namespace MikuruPrivate {

class Part;
class EventDispatcher;

class Filter:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

	friend class Patch;

  public:
	enum FilterID { Filter1, Filter2 };

  public:
	/**
	 * \param	part can be null, if this is common filter.
	 */
	Filter (FilterID filter_id, Haruhi::PortGroup* port_group, QString const& port_prefix, QString const& label, Part* part, Mikuru* mikuru, QWidget* parent);

	~Filter();

	Params::Filter*
	params() { return &_params; }

	void
	process_events();

	void
	unit_bay_assigned();

  public slots:
	/**
	 * Loads widgets values from Params struct.
	 */
	void
	load_params();

	/**
	 * Loads params from given struct and updates widgets.
	 */
	void
	load_params (Params::Filter& params);

	/**
	 * Updates Params structure from widgets.
	 */
	void
	update_params();

	/**
	 * Updates widgets.
	 */
	void
	update_widgets();

	/**
	 * Redraws frequency response.
	 */
	void
	update_impulse_response();

  private:
	/**
	 * Sets up params callbacks.
	 */
	void
	setup_params();

  private:
	Mikuru*							_mikuru;
	Params::Filter					_params;
	bool							_loading_params;
	Part*							_part;
	RBJImpulseResponse				_impulse_response;
	bool							_polyphonic_control;

	// Ports:
	Haruhi::EventPort*				_port_frequency;
	Haruhi::EventPort*				_port_resonance;
	Haruhi::EventPort*				_port_gain;
	Haruhi::EventPort*				_port_attenuation;

	// Event dispatchers for polyphonic-input ports:
	EventDispatcher*				_evdisp_frequency;
	EventDispatcher*				_evdisp_resonance;
	EventDispatcher*				_evdisp_gain;
	EventDispatcher*				_evdisp_attenuation;
	FilterID						_filter_id;

	// Enabled:
	StyledCheckBoxLabel*			_filter_label;

	// Widgets and knobs:
	QWidget*						_panel;
	Haruhi::FrequencyResponsePlot*	_response_plot;
	QComboBox*						_filter_type;
	QSpinBox*						_stages;
	QCheckBox*						_limiter_enabled;
	Haruhi::Knob*					_knob_frequency;
	Haruhi::Knob*					_knob_resonance;
	Haruhi::Knob*					_knob_gain;
	Haruhi::Knob*					_knob_attenuation;
};

} // namespace MikuruPrivate

#endif

