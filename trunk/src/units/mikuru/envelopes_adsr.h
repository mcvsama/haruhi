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
#include <haruhi/graph/event.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/adsr.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/lib/controller_proxy.h>
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
	Mikuru*					_mikuru;
	Params::ADSR			_params;
	bool					_loading_params;
	int						_id;
	DSP::Envelope			_envelope_for_plot;
	ADSRs					_adsrs;
	Haruhi::AudioBuffer		_buffer;
	// List of Voices which has been dropped and need ADSRs to be deleted also:
	std::list<Voice*>		_dropped_voices;

	Haruhi::PortGroup*		_port_group;
	Haruhi::EventPort*		_port_delay;
	Haruhi::EventPort*		_port_attack;
	Haruhi::EventPort*		_port_attack_hold;
	Haruhi::EventPort*		_port_decay;
	Haruhi::EventPort*		_port_sustain;
	Haruhi::EventPort*		_port_sustain_hold;
	Haruhi::EventPort*		_port_release;
	Haruhi::EventPort*		_port_output;

	QCheckBox*				_enabled;
	QCheckBox*				_direct_adsr;
	QCheckBox*				_forced_release;
	QCheckBox*				_sustain_enabled;
	Haruhi::Knob*			_knob_delay;
	Haruhi::Knob*			_knob_attack;
	Haruhi::Knob*			_knob_attack_hold;
	Haruhi::Knob*			_knob_decay;
	Haruhi::Knob*			_knob_sustain;
	Haruhi::Knob*			_knob_sustain_hold;
	Haruhi::Knob*			_knob_release;
	QComboBox*				_function;
	QComboBox*				_mode;
	Haruhi::EnvelopePlot*	_plot;
};

} // namespace MikuruPrivate

#endif

