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

#ifndef HARUHI__PLUGINS__MIKURU__ENVELOPES_EG_H__INCLUDED
#define HARUHI__PLUGINS__MIKURU__ENVELOPES_EG_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>
#include <list>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/graph/event.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/envelope.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/widgets/envelope_plot.h>
#include <haruhi/utility/mutex.h>

// Local:
#include "../params.h"
#include "envelope.h"


class Mikuru;

namespace MikuruPrivate {

namespace DSP = Haruhi::DSP;
class Voice;
class VoiceManager;
class Part;


class EG: public Envelope
{
	Q_OBJECT

	friend class Patch;

	enum { ARTIFICIAL_SAMPLE_RATE = 1000000 };

	typedef std::map<Voice*, DSP::Envelope*> EGs;

  public:
	EG (int id, Mikuru* mikuru, QWidget* parent);

	~EG();

  private:
	void
	create_ports();

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

	/**
	 * Returns copy of envelope used by EG.
	 * \threadsafe
	 */
	DSP::Envelope
	envelope_template();

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

  private slots:
	/**
	 * Updates widgets dependencies.
	 */
	void
	update_widgets();

	void
	changed_active_point();

	void
	changed_segment_value();

	void
	changed_segment_duration();

	void
	changed_envelope();

	void
	add_point_before_active();

	void
	add_point_after_active();

	void
	remove_active_point();

  private:
	/**
	 * Delete EGs for Voices registered as dropped.
	 */
	void
	sweep();

	/**
	 * Updates value/duration knobs from current envelope settings
	 * and active point index.
	 */
	void
	update_point_knobs();

  private:
	Mikuru*						_mikuru;
	Params::EG					_params;
	bool						_loading_params;
	bool						_updating_widgets;
	// Set to true to stop point controls changing envelope:
	bool						_mute_point_controls;
	int							_id;
	// Segment lengths in template envelope are measured for artificial
	// sample-rate: ARTIFICIAL_SAMPLE_RATE samples/sec. Therefore when creating real envelope
	// all segment lengths must be recomputed to match current sample-rate.
	DSP::Envelope				_envelope_template;
	Mutex						_envelope_template_mutex;
	EGs							_egs;
	Haruhi::AudioBuffer			_buffer;
	// List of Voices which has been dropped and need ADSRs to be deleted also:
	std::list<Voice*>			_dropped_voices;

	Haruhi::ControllerParam		_segment_duration;
	Haruhi::ControllerParam		_point_value;

	Haruhi::PortGroup*			_port_group;
	Haruhi::EventPort*			_port_point_value;
	Haruhi::EventPort*			_port_segment_duration;
	Haruhi::EventPort*			_port_output;

	QCheckBox*					_enabled;
	Haruhi::Knob*				_knob_point_value;
	Haruhi::Knob*				_knob_segment_duration;
	QSpinBox*					_active_point;
	QSpinBox*					_sustain_point;
	QPushButton*				_add_point_before_active;
	QPushButton*				_add_point_after_active;
	QPushButton*				_remove_active_point;
	Haruhi::EnvelopePlot*		_plot;
};

} // namespace MikuruPrivate

#endif

