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

#ifndef HARUHI__PLUGINS__FREEVERB__ALLPASS_FILTER_H__INCLUDED
#define HARUHI__PLUGINS__FREEVERB__ALLPASS_FILTER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>


namespace Freeverb {

class AllpassFilter
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
	 * Set feedback parameter.
	 */
	void
	set_feedback (float value) noexcept;

  private:
	float				_feedback	= 0.0;
	Haruhi::AudioBuffer	_buffer;
	size_t				_pos		= 0;
};


inline void
AllpassFilter::set_buffer_size (size_t size)
{
	_buffer.resize (size);
}


inline Haruhi::Sample
AllpassFilter::process (Haruhi::Sample input)
{
	Haruhi::Sample output;
	Haruhi::Sample bufout;

	bufout = _buffer[_pos];
	undenormalize (bufout);

	output = -input + bufout;
	_buffer[_pos] = input + bufout * _feedback;

	if (++_pos >= _buffer.size())
		_pos = 0;

	return output;
}


inline void
AllpassFilter::clear()
{
	_buffer.clear();
}


inline void
AllpassFilter::set_feedback (float value) noexcept
{
	_feedback = value;
}

} // namespace Freeverb

#endif

