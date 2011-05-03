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

#ifndef HARUHI__UNITS__MIKURU__EFFECTS__REVERB_H__INCLUDED
#define HARUHI__UNITS__MIKURU__EFFECTS__REVERB_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QPushButton>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "effect.h"


class Mikuru;

namespace MikuruPrivate {

using Haruhi::Sample;

/**
 * Freeverb implementation. Based on code by Jezar at Dreampoint.
 */
class Reverb: public Effect
{
	Q_OBJECT

	friend class Patch;

	class CombFilter
	{
	  public:
		CombFilter();

		void
		set_buffer_size (size_t size);

		Sample
		process (Sample input)
		{
			Sample output;

			output = _buffer[_pos];
			undenormalize (output);

			_filter_store = output * _damp2 + _filter_store * _damp1;
			undenormalize (_filter_store);

			_buffer[_pos] = input + _filter_store * _feedback;

			if (++_pos >= _buffer.size())
				_pos = 0;

			return output;
		}

		void
		clear();

		void
		set_damp (float value);

		float
		get_damp() { return _damp1; }

		void
		set_feedback (float value) { _feedback = value; }

		float
		get_feedback() { return _feedback; }

	  private:
		float				_feedback;
		float				_filter_store;
		float				_damp1;
		float				_damp2;
		Haruhi::AudioBuffer	_buffer;
		size_t				_pos;
	};

	class AllpassFilter
	{
	  public:
		AllpassFilter();

		void
		set_buffer_size (size_t size);

		Sample
		process (Sample input)
		{
			Sample output;
			Sample bufout;

			bufout = _buffer[_pos];
			undenormalize (bufout);

			output = -input + bufout;
			_buffer[_pos] = input + bufout * _feedback;

			if (++_pos >= _buffer.size())
				_pos = 0;

			return output;
		}

		void
		clear();

		void
		set_feedback (float value) { _feedback = value; }

		float
		get_feedback() { return _feedback; }

	  private:
		float				_feedback;
		Haruhi::AudioBuffer	_buffer;
		size_t				_pos;
	};

	class Model
	{
		static const int	NumCombs		= 8;
		static const int	NumAllpasses	= 4;
		static const float	FixedGain		= 0.015f;
		static const float	ScaleDamp		= 0.4f;
		static const float	ScaleRoom		= 0.28f;
		static const float	OffsetRoom		= 0.7f;
		static const float	InitialRoom		= 0.5f;
		static const float	InitialDamp		= 0.5f;
		static const float	InitialWidth	= 1.0f;
		static const float	InitialMode		= 0.0f;
		static const int	StereoSpread	= 23;

		// These values assume 44.1KHz sample rate
		// they will probably be OK for 48KHz sample rate
		// but would need scaling for 96KHz (or other) sample rates.
		// The values were obtained by listening tests.
		static const int	CombTuningL1	= 1116;
		static const int	CombTuningR1	= 1116 + StereoSpread;
		static const int	CombTuningL2	= 1188;
		static const int	CombTuningR2	= 1188 + StereoSpread;
		static const int	CombTuningL3	= 1277;
		static const int	CombTuningR3	= 1277 + StereoSpread;
		static const int	CombTuningL4	= 1356;
		static const int	CombTuningR4	= 1356 + StereoSpread;
		static const int	CombTuningL5	= 1422;
		static const int	CombTuningR5	= 1422 + StereoSpread;
		static const int	CombTuningL6	= 1491;
		static const int	CombTuningR6	= 1491 + StereoSpread;
		static const int	CombTuningL7	= 1557;
		static const int	CombTuningR7	= 1557 + StereoSpread;
		static const int	CombTuningL8	= 1617;
		static const int	CombTuningR8	= 1617 + StereoSpread;
		static const int	AllpassTuningL1	= 556;
		static const int	AllpassTuningR1	= 556 + StereoSpread;
		static const int	AllpassTuningL2	= 441;
		static const int	AllpassTuningR2	= 441 + StereoSpread;
		static const int	AllpassTuningL3	= 341;
		static const int	AllpassTuningR3	= 341 + StereoSpread;
		static const int	AllpassTuningL4	= 225;
		static const int	AllpassTuningR4	= 225 + StereoSpread;

	  public:
		enum Mode {
			NormalMode,
			FreezeMode
		};

	  public:
		Model (unsigned int sample_rate);

		void
		clear();

		void
		process (Sample* inputL, Sample* inputR, Sample* outputL, Sample* outputR, size_t samples);

		void
		set_room_size (float value);

		void
		set_damp (float value);

		void
		set_width (float value);

		void
		set_mode (Mode mode);

		/**
		 * Notifies about new sample rate, so model can recompute
		 * its internal values for the new value.
		 */
		void
		set_sample_rate (unsigned int sample_rate);

	  private:
		/**
		 * Resize buffers according to new sample rate, since
		 * filters' params depend on it.
		 */
		void
		scale_buffers (unsigned int sample_rate);

		/**
		 * Recomputes internal values after parameter change.
		 */
		void
		update();

	  private:
		float			_gain;
		float			_room_size;
		float			_room_size1;
		float			_damp;
		float			_damp1;
		float			_wet1;
		float			_wet2;
		float			_width;
		Mode			_mode;

		// Comb filters:
		CombFilter		_combL[NumCombs];
		CombFilter		_combR[NumCombs];

		// Allpass filters:
		AllpassFilter	_allpassL[NumAllpasses];
		AllpassFilter	_allpassR[NumAllpasses];
	};

  public:
	Reverb (int id, Mikuru* mikuru, QWidget* parent);

	~Reverb();

	Params::Reverb*
	params() const { return _params; }

	void
	graph_updated();

	void
	process_events();

	void
	process (Haruhi::AudioBuffer* in1, Haruhi::AudioBuffer* in2, Haruhi::AudioBuffer* out1, Haruhi::AudioBuffer* out2);

	void
	panic();

  public slots:
	/**
	 * Loads widgets values from Params struct.
	 */
	void
	load_params();

	void
	load_params (Params::Reverb& params);

	/**
	 * Updates Params structure from widgets.
	 */
	void
	update_params();

  private:
	Mikuru*				_mikuru;
	Params::Reverb*		_params;
	bool				_loading_params;
	Model				_reverb_model;
	QPushButton*		_freeze_mode;

	Haruhi::Knob*		_knob_room_size;
	Haruhi::Knob*		_knob_width;
	Haruhi::Knob*		_knob_damp;

	Haruhi::EventPort*	_port_room_size;
	Haruhi::EventPort*	_port_width;
	Haruhi::EventPort*	_port_damp;
};

} // namespace MikuruPrivate

#endif

