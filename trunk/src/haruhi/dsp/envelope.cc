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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/utility/numeric.h>

// Local:
#include "envelope.h"


namespace Haruhi {

namespace DSP {

Envelope::Envelope():
	_phase (0),
	_p (0),
	_sustain_point (0),
	_sustain (false),
	_forced_release (false)
{
}


void
Envelope::prepare()
{
	_phase = 0;
	_p = 0;
	_sustain = _phase == _sustain_point;
}


void
Envelope::release()
{
	_points[_sustain_point].value = renormalize (1.0f * _p, 0.0f, 1.0f * _points[_phase].samples, _points[_phase].value, _points[_phase+1].value);
	_p = 0;
	_phase = _sustain_point;
	_sustain = false;
}


bool
Envelope::released() const
{
	return _phase >= _sustain_point && !_sustain;
}


bool
Envelope::finished() const
{
	return !_sustain && _phase >= _points.size() - 1;
}


void
Envelope::fill (Core::Sample* begin, Core::Sample* end)
{
	Core::Sample* cur = begin;
	while (cur != end)
	{
		if (_sustain)
			*cur++ = _points[_phase].value;
		else
		{
			if (_phase < _points.size() - 1)
			{
				unsigned int samples = _points[_phase].samples;
				float v1 = _points[_phase].value;
				float v2 = _points[_phase+1].value;
				// Fill samples:
				while (cur != end && _p < samples)
				{
					*cur++ = renormalize (1.0f * _p, 0.0f, 1.0f * samples, v1, v2);
					_p += 1;
				}
				// If phase finished:
				if (_p >= _points[_phase].samples)
				{
					_p = 0;
					_phase = std::min (static_cast<Points::size_type> (_phase + 1), _points.size()-1);
					if (_phase == _sustain_point)
						_sustain = true;
				}
			}
			else
				while (cur != end)
					*cur++ = _points.back().value;
		}
	}
}

} // namespace DSP

} // namespace Haruhi

