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

#ifndef HARUHI__PLUGINS__FREEVERB__COMB_FILTER_H__INCLUDED
#define HARUHI__PLUGINS__FREEVERB__COMB_FILTER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/utility/numeric.h>


namespace Freeverb {

class CombFilter
{
  public:
	/**
	 * Resize buffers.
	 */
	void
	set_buffer_size (size_t size);

	/**
	 * Process single sample.
	 */
	Haruhi::Sample
	process (Haruhi::Sample input);

	/**
	 * Clear audio buffer.
	 */
	void
	clear();

	/**
	 * Set damping parameter.
	 */
	void
	set_damping (float value) noexcept;

	/**
	 * Set feedback parameter.
	 */
	void
	set_feedback (float value) noexcept;

  private:
	float				_feedback		= 0.0;
	float				_filter_store	= 0.0;
	float				_damp1			= 0.0;
	float				_damp2			= 1.0;
	Haruhi::AudioBuffer	_buffer;
	size_t				_pos			= 0;
};


inline void
CombFilter::set_buffer_size (size_t size)
{
	_buffer.resize (size);
}


inline Haruhi::Sample
CombFilter::process (Haruhi::Sample input)
{
	Haruhi::Sample output;

	output = _buffer[_pos];
	undenormalize (output);

	_filter_store = output * _damp2 + _filter_store * _damp1;
	undenormalize (_filter_store);

	_buffer[_pos] = input + _filter_store * _feedback;

	if (++_pos >= _buffer.size())
		_pos = 0;

	return output;
}


inline void
CombFilter::clear()
{
	_buffer.clear();
}


inline void
CombFilter::set_damping (float value) noexcept
{
	_damp1 = value;
	_damp2 = 1.0f - value;
}


inline void
CombFilter::set_feedback (float value) noexcept
{
	_feedback = value;
}

} // namespace Freeverb

#endif

