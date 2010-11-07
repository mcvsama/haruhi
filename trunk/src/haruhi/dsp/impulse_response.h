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

#ifndef HARUHI__DSP__IMPULSE_RESPONSE_H__INCLUDED
#define HARUHI__DSP__IMPULSE_RESPONSE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Haruhi:
#include <haruhi/core/audio.h>


namespace Haruhi {

namespace DSP {

/**
 * Base class for impulse responses.
 */
class ImpulseResponse
{
  public:
	typedef unsigned int Serial;

  public:
	ImpulseResponse():
		_serial (0)
	{ }

	virtual ~ImpulseResponse() { }

	/**
	 * Returns size of IR vector.
	 */
	virtual int
	size() const = 0;

	/**
	 * Computes response magnitude for given frequency (0, 0.5).
	 */
	virtual Core::Sample
	response (Core::Sample frequency) const = 0;

	/**
	 * Returns table of H(z)'s denominator coefficients.
	 * For FIR returns 0.
	 */
	virtual Core::Sample*
	a() = 0;

	/**
	 * Returns table of H(z)'s numerator coefficients.
	 */
	virtual Core::Sample*
	b() = 0;

	/**
	 * Returns current serial number.
	 * Serial number changes whenever impulse response
	 * coefficients change.
	 */
	Serial
	serial() const { return _serial; }

  protected:
	/**
	 * Increases serial number, so filter will
	 * know to reset recursive coefficients.
	 */
	void
	bump() { ++_serial; }

  public:
	Serial _serial;
};

} // namespace DSP

} // namespace Haruhi

#endif

