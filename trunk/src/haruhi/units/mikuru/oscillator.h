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

#ifndef HARUHI__UNITS__MIKURU__OSCILLATOR_H__INCLUDED
#define HARUHI__UNITS__MIKURU__OSCILLATOR_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/controller_proxy.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/core/port_group.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "event_dispatcher.h"
#include "widgets.h"
#include "params.h"


class Mikuru;

namespace MikuruPrivate {

class Part;

class Oscillator: public QWidget
{
	Q_OBJECT

	friend class Patch;

  public:
	Oscillator (Part* part, Core::PortGroup* port_group, QString const& port_prefix, Mikuru* mikuru, QWidget* parent);

	~Oscillator();

	Params::Oscillator*
	oscillator_params() { return &_oscillator_params; }

	Params::Voice*
	voice_params() { return &_voice_params; }

	void
	process_events();

	Core::EventPort*
	pitchbend_port() const { return _port_pitchbend; }

	Core::EventPort*
	amplitude_port() const { return _port_amplitude; }

	Core::EventPort*
	frequency_port() const { return _port_frequency; }

  public slots:
	/**
	 * Loads widgets values from Params struct.
	 */
	void
	load_oscillator_params();

	void
	load_voice_params();

	void
	update_params()
	{
		update_oscillator_params();
		update_voice_params();
	}

	/**
	 * Loads params from given struct and updates widgets.
	 */
	void
	load_oscillator_params (Params::Oscillator& params);

	void
	load_voice_params (Params::Voice& params);

	/**
	 * Updates Params structure from widgets.
	 */
	void
	update_oscillator_params();

	void
	update_voice_params();

	/**
	 * Updates widgets.
	 */
	void
	update_widgets();

  private slots:
	void
	update_voice_panorama();

	void
	update_voice_detune();

	void
	update_voice_pitchbend();

	void
	update_voice_unison_index();

	void
	update_voice_unison_spread();

	void
	update_voice_unison_init();

	void
	update_voice_unison_noise();

	void
	update_voice_velocity_sens();

  private:
	Mikuru*				_mikuru;
	Part*				_part;
	Params::Oscillator	_oscillator_params;
	Params::Voice		_voice_params;
	bool				_loading_params;
	QWidget*			_panel;

	// Part ports:
	Core::EventPort*	_port_volume;
	Core::EventPort*	_port_portamento_time;
	Core::EventPort*	_port_phase;

	// Polyphonic-input ports:
	Core::EventPort*	_port_amplitude;
	Core::EventPort*	_port_frequency;
	Core::EventPort*	_port_panorama;
	Core::EventPort*	_port_detune;
	Core::EventPort*	_port_pitchbend;
	Core::EventPort*	_port_velocity_sens;
	Core::EventPort*	_port_unison_index;
	Core::EventPort*	_port_unison_spread;
	Core::EventPort*	_port_unison_init;
	Core::EventPort*	_port_unison_noise;

	// Proxies:
	ControllerProxy*	_proxy_volume;
	ControllerProxy*	_proxy_portamento_time;
	ControllerProxy*	_proxy_phase;
	ControllerProxy*	_proxy_panorama;
	ControllerProxy*	_proxy_detune;
	ControllerProxy*	_proxy_pitchbend;
	ControllerProxy*	_proxy_velocity_sens;
	ControllerProxy*	_proxy_unison_index;
	ControllerProxy*	_proxy_unison_spread;
	ControllerProxy*	_proxy_unison_init;
	ControllerProxy*	_proxy_unison_noise;

	// Event dispatchers for polyphonic-input ports:
	EventDispatcher*	_evdisp_amplitude;
	EventDispatcher*	_evdisp_frequency;
	EventDispatcher*	_evdisp_panorama;
	EventDispatcher*	_evdisp_detune;
	EventDispatcher*	_evdisp_pitchbend;
	EventDispatcher*	_evdisp_velocity_sens;
	EventDispatcher*	_evdisp_unison_index;
	EventDispatcher*	_evdisp_unison_spread;
	EventDispatcher*	_evdisp_unison_init;
	EventDispatcher*	_evdisp_unison_noise;

	// Volume knobs:
	Knob*				_control_volume;
	Knob*				_control_panorama;
	Knob*				_control_detune;
	Knob*				_control_pitchbend;
	Knob*				_control_velocity_sens;

	// Unison knobs:
	Knob*				_control_unison_index;
	Knob*				_control_unison_spread;
	Knob*				_control_unison_init;
	Knob*				_control_unison_noise;

	// Other knobs:
	Knob*				_control_portamento_time;
	Knob*				_control_phase;

	// Pitchbend/transposition:
	QCheckBox*			_const_portamento_time;
	QCheckBox*			_pitchbend_enabled;
	QCheckBox*			_pitchbend_released;
	QSpinBox*			_pitchbend_up_semitones;
	QSpinBox*			_pitchbend_down_semitones;
	QSpinBox*			_transposition_semitones;
	QSpinBox*			_frequency_modulation_range;
	QSpinBox*			_amplitude_modulation_smoothing;
	QSpinBox*			_frequency_modulation_smoothing;

	// Monophonic:
	QCheckBox*			_monophonic;
	QCheckBox*			_monophonic_retrigger;
	QComboBox*			_monophonic_key_priority;
};

} // namespace MikuruPrivate

#endif

