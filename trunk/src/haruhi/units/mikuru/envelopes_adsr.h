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

#ifndef HARUHI__UNITS__MIKURU__ENVELOPES_ADSR_H__INCLUDED
#define HARUHI__UNITS__MIKURU__ENVELOPES_ADSR_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>
#include <list>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/controller_proxy.h>
#include <haruhi/core/event.h>
#include <haruhi/dsp/adsr.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/widgets/envelope_plot.h>

// Local:
#include "envelopes.h"
#include "params.h"


class Mikuru;

namespace MikuruPrivate {

class Voice;
class VoiceManager;
class Part;


class ADSR: public Envelope
{
	Q_OBJECT

	friend class Patch;

	typedef std::map<Voice*, DSP::ADSR*> ADSRs;

  public:
	ADSR (int id, Mikuru* mikuru, QWidget* parent);

	~ADSR();

  private:
	void
	create_ports();

	void
	create_proxies();

	void
	create_knobs (QWidget* parent);

	void
	create_widgets (QWidget* knobs_panel);

  public:
	Params::ADSR*
	params() { return &_params; }

	int
	id() const { return _id; }

	void
	voice_created (VoiceManager*, Voice*);

	void
	voice_released (VoiceManager*, Voice*);

	void
	voice_dropped (VoiceManager*, Voice*);

	void
	process();

	void
	resize_buffers (std::size_t size);

  public slots:
	/**
	 * Loads widgets values from Params struct.
	 */
	void
	load_params();

	void
	load_params (Params::ADSR& params);

	/**
	 * Updates Params structure from widgets.
	 */
	void
	update_params();

	/**
	 * Updates envelope plot.
	 */
	void
	update_plot();

  private:
	/**
	 * Delete ADSRs for Voices registered as dropped.
	 */
	void
	sweep();

  private:
	Mikuru*						_mikuru;
	Params::ADSR				_params;
	bool						_loading_params;
	int							_id;
	DSP::Envelope				_envelope_for_plot;
	ADSRs						_adsrs;
	Core::AudioBuffer			_buffer;
	// List of Voices which has been dropped and need ADSRs to be deleted also:
	std::list<Voice*>			_dropped_voices;

	Core::PortGroup*			_port_group;
	Core::EventPort*			_port_delay;
	Core::EventPort*			_port_attack;
	Core::EventPort*			_port_attack_hold;
	Core::EventPort*			_port_decay;
	Core::EventPort*			_port_sustain;
	Core::EventPort*			_port_sustain_hold;
	Core::EventPort*			_port_release;
	Core::EventPort*			_port_output;

	Haruhi::ControllerProxy*	_proxy_delay;
	Haruhi::ControllerProxy*	_proxy_attack;
	Haruhi::ControllerProxy*	_proxy_attack_hold;
	Haruhi::ControllerProxy*	_proxy_decay;
	Haruhi::ControllerProxy*	_proxy_sustain;
	Haruhi::ControllerProxy*	_proxy_sustain_hold;
	Haruhi::ControllerProxy*	_proxy_release;

	QCheckBox*					_enabled;
	QCheckBox*					_direct_adsr;
	QCheckBox*					_forced_release;
	QCheckBox*					_sustain_enabled;
	Haruhi::Knob*				_control_delay;
	Haruhi::Knob*				_control_attack;
	Haruhi::Knob*				_control_attack_hold;
	Haruhi::Knob*				_control_decay;
	Haruhi::Knob*				_control_sustain;
	Haruhi::Knob*				_control_sustain_hold;
	Haruhi::Knob*				_control_release;
	QComboBox*					_function;
	QComboBox*					_mode;
	Haruhi::EnvelopePlot*		_plot;
};

} // namespace MikuruPrivate

#endif

