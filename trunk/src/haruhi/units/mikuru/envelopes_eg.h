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

#ifndef HARUHI__UNITS__MIKURU__ENVELOPES_ENV_H__INCLUDED
#define HARUHI__UNITS__MIKURU__ENVELOPES_ENV_H__INCLUDED

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
#include <haruhi/dsp/envelope.h>
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


class EG: public Envelope
{
	Q_OBJECT

	friend class Patch;

	typedef std::map<Voice*, DSP::Envelope*> EGs;

	enum { SR_FOR_TEMPLATE = 100000 };

  public:
	EG (int id, Mikuru* mikuru, QWidget* parent);

	~EG();

  private:
	void
	create_ports();

	void
	create_proxies();

	void
	create_knobs (QWidget* parent);

	void
	create_widgets();

  public:
	Params::EG*
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
	load_params (Params::EG& params);

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
	 * Delete EGs for Voices registered as dropped.
	 */
	void
	sweep();

  private:
	Mikuru*					_mikuru;
	Params::EG				_params;
	bool					_loading_params;
	int						_id;
	// It's assumed that sample rate for _envelope_template is SR_FOR_TEMPLATE.
	// Envelopes created from this template require segment lengths adjustment
	// according to real current sample rate.
	DSP::Envelope			_envelope_template;
	EGs						_egs;
	Core::AudioBuffer		_buffer;
	// List of Voices which has been dropped and need ADSRs to be deleted also:
	std::list<Voice*>		_dropped_voices;

	Core::PortGroup*		_port_group;
	Core::EventPort*		_port_segment_duration;

	ControllerProxy*		_proxy_segment_duration;

	QCheckBox*				_enabled;
	Knob*					_control_segment_duration;
	EnvelopePlot*			_plot;
	QSpinBox*				_sustain_point;
	QSpinBox*				_total_points;
};

} // namespace MikuruPrivate

#endif

