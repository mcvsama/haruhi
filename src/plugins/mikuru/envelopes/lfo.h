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

#ifndef HARUHI__UNITS__MIKURU__ENVELOPES_LFO_H__INCLUDED
#define HARUHI__UNITS__MIKURU__ENVELOPES_LFO_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>
#include <map>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/dsp/parametric_wave.h>
#include <haruhi/dsp/noise.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/widgets/wave_plot.h>

// Local:
#include "../params.h"
#include "envelope.h"


class Mikuru;

namespace MikuruPrivate {

namespace DSP = Haruhi::DSP;
class Voice;
class VoiceManager;
class Part;


class LFO: public Envelope
{
	Q_OBJECT

	friend class Patch;

	class Osc
	{
	  public:
		Osc (float start_phase = 0);

		void
		reset (float start_phase);

		void
		release() { _released = true; }

		/**
		 * \param	wave is Wave object to use.
		 */
		void
		set_wave (DSP::ParametricWave* wave, bool is_random) { _wave = wave; _wave_is_random = is_random; }

		/**
		 * \param	delay is delay in samples.
		 */
		void
		set_delay (unsigned int samples) { _delay_samples = samples; }

		/**
		 * \param	fade_in is fade in after delay.
		 */
		void
		set_fade_in (unsigned int samples) { _fade_in_samples = samples; }

		/**
		 * \param	fade_out is fade out after fade_in.
		 */
		void
		set_fade_out (unsigned int samples) { _fade_out_samples = samples; }

		void
		set_fade_out_enabled (bool enabled) { _fade_out_enabled = enabled; }

		/**
		 * \param	frequency is absolute frequency [0..1].
		 */
		void
		set_frequency (float frequency) { _frequency = frequency; }

		/**
		 * \param	level is center level [0..1].
		 */
		void
		set_level (float level) { _level = level; }

		/**
		 * \param	depth is intensity of oscillations [0..1].
		 */
		void
		set_depth (float depth) { _depth = depth; }

		/**
		 * \param	invert enables or disables wave value inversion.
		 */
		void
		set_invert (bool invert) { _inverter = invert ? -1.0f : 1.0f; }

		/**
		 * \param	samples is number of samples to advance.
		 * \returns	new sample after advancing LFO a number of samples.
		 */
		Haruhi::Sample
		advance (unsigned int samples);

	  private:
		DSP::ParametricWave*	_wave;
		bool					_wave_is_random;
		float					_phase;
		unsigned int			_delay_samples;
		unsigned int			_fade_in_samples;
		unsigned int			_fade_out_samples;
		bool					_fade_out_enabled;
		float					_frequency;
		float					_level;
		float					_depth;
		float					_inverter;
		unsigned int			_current_delay_sample;
		unsigned int			_current_fade_in_sample;
		unsigned int			_current_fade_out_sample;
		bool					_released;
	};

	class RandomWave: public DSP::ParametricWave
	{
	  public:
		enum Type { Square, Triangle };

	  public:
		RandomWave (Type);

		/**
		 * Tells wave to switch to next step
		 * in random flow.
		 */
		void
		next_step();

		/*
		 * DSP::Wave API.
		 */

		Haruhi::Sample
		operator() (Haruhi::Sample register phase, Haruhi::Sample frequency) const;

	  private:
		float
		noise_sample() { return _noise.get (_noise_state); }

	  private:
		Type				_type;
		float				_prev_value;
		float				_curr_value;
		DSP::Noise			_noise;
		DSP::Noise::State	_noise_state;
	};

	typedef std::map<Voice*, Osc*> Oscs;
	typedef std::map<int, DSP::ParametricWave*> Waves;

  public:
	LFO (int id, Mikuru* mikuru, QWidget* parent);

	~LFO();

  private:
	void
	create_ports();

	void
	create_knobs (QWidget* parent);

	void
	create_widgets (QWidget* knobs_panel);

  public:
	Params::LFO*
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
	load_params (Params::LFO& params);

	/**
	 * Updates Params structure from widgets.
	 */
	void
	update_params();

	/**
	 * Updates wave plot.
	 */
	void
	update_plot();

	/**
	 * Updates widget states.
	 */
	void
	update_widgets();

	/**
	 * Sets currently selected wave's param.
	 */
	void
	update_wave_param();

  private:
	void
	reset_common_osc();

	void
	set_common_osc();

	Haruhi::Sample
	apply_function (Haruhi::Sample v) const;

	float
	get_phase() const;

  private:
	Mikuru*				_mikuru;
	Params::LFO			_params;
	bool				_loading_params;
	int					_id;
	int					_pressed_keys;

	Haruhi::PortGroup*	_port_group;
	Haruhi::EventPort*	_port_delay;
	Haruhi::EventPort*	_port_fade_in;
	Haruhi::EventPort*	_port_frequency;
	Haruhi::EventPort*	_port_level;
	Haruhi::EventPort*	_port_depth;
	Haruhi::EventPort*	_port_phase;
	Haruhi::EventPort*	_port_output;
	Haruhi::EventPort*	_port_wave_shape;
	Haruhi::EventPort*	_port_fade_out;

	Oscs				_oscs;
	Osc					_common_osc;
	Waves				_waves;
	DSP::Noise			_noise;

	Haruhi::Knob*		_knob_delay;
	Haruhi::Knob*		_knob_fade_in;
	Haruhi::Knob*		_knob_frequency;
	Haruhi::Knob*		_knob_level;
	Haruhi::Knob*		_knob_depth;
	Haruhi::Knob*		_knob_phase;
	Haruhi::Knob*		_knob_wave_shape;
	Haruhi::Knob*		_knob_fade_out;

	QCheckBox*			_enabled;
	QComboBox*			_wave_type;
	QCheckBox*			_wave_invert;
	QComboBox*			_function;
	QComboBox*			_mode;
	QCheckBox*			_tempo_sync;
	QSpinBox*			_tempo_numerator;
	QSpinBox*			_tempo_denominator;
	QCheckBox*			_random_start_phase;
	QCheckBox*			_fade_out_enabled;
	Haruhi::WavePlot*	_plot;
};

} // namespace MikuruPrivate

#endif

