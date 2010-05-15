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

#ifndef HARUHI__DSP__ENVELOPE_H__INCLUDED
#define HARUHI__DSP__ENVELOPE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>

// Haruhi:
#include <haruhi/core/audio.h>


namespace DSP {

// TODO make multi-sustaining (many points may be sustain points)
class Envelope
{
  public:
	class Point
	{
	  public:
		Point():
			value (0.0),
			samples (0)
		{ }

		Point (float value, unsigned int samples, bool sustain = false):
			value (value),
			samples (samples),
			sustain (sustain)
		{ }

	  public:
		float			value;
		unsigned int	samples;
		bool			sustain;
	};

	typedef std::vector<Point>  Points;

  public:
	Envelope();

	Points::size_type
	release_point() const { return _release_point; }

	void
	set_release_point (Points::size_type i) { _release_point = i; points()[i].sustain = true; }

	/**
	 * If forced release is enabled, envelope will jump directly to
	 * release point on release.
	 */
	void
	set_forced_release (bool enabled) { _forced_release = enabled; }

	Points&
	points() { return _points; }

	Points const&
	points() const { return _points; }

	void
	prepare();

	void
	release();

	bool
	released() const;

	bool
	finished() const;

	void
	fill (Core::Sample* begin, Core::Sample* end);

  private:
	Points				_points;
	unsigned int		_phase;
	unsigned int		_p;
	Points::size_type	_release_point;
	bool				_sustain;
	bool				_forced_release;
};

} // namespace DSP

#endif

