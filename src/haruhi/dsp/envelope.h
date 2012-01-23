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

#ifndef HARUHI__DSP__ENVELOPE_H__INCLUDED
#define HARUHI__DSP__ENVELOPE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

namespace DSP {

class Envelope
{
  public:
	class Point
	{
	  public:
		Point();

		Point (float value, unsigned int samples);

	  public:
		float			value;
		unsigned int	samples;
	};

	typedef std::vector<Point>  Points;

  public:
	Envelope() noexcept;

	Points::size_type
	sustain_point() const noexcept;

	void
	set_sustain_point (Points::size_type i) noexcept;

	/**
	 * If forced release is enabled, envelope will jump directly to
	 * release point on release.
	 */
	void
	set_forced_release (bool enabled) noexcept;

	Points&
	points() noexcept;

	Points const&
	points() const noexcept;

	void
	prepare() noexcept;

	void
	release() noexcept;

	bool
	released() const noexcept;

	bool
	finished() const noexcept;

	void
	fill (Sample* begin, Sample* end) noexcept;

  private:
	Points				_points;
	unsigned int		_phase;
	unsigned int		_p;
	Points::size_type	_sustain_point;
	bool				_sustain;
	bool				_forced_release;
};


inline
Envelope::Point::Point():
	value (0.0),
	samples (0)
{ }


inline
Envelope::Point::Point (float value, unsigned int samples):
	value (value),
	samples (samples)
{ }


inline Envelope::Points::size_type
Envelope::sustain_point() const noexcept
{
	return _sustain_point;
}


inline void
Envelope::set_sustain_point (Points::size_type i) noexcept
{
	_sustain_point = i;
}


inline void
Envelope::set_forced_release (bool enabled) noexcept
{
	_forced_release = enabled;
}


inline Envelope::Points&
Envelope::points() noexcept
{
	return _points;
}


inline Envelope::Points const&
Envelope::points() const noexcept
{
	return _points;
}


inline bool
Envelope::released() const noexcept
{
	return _phase >= _sustain_point && !_sustain;
}


inline bool
Envelope::finished() const noexcept
{
	return !_sustain && _phase >= _points.size() - 1;
}

} // namespace DSP

} // namespace Haruhi

#endif

