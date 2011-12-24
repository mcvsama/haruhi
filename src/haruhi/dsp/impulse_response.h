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

#ifndef HARUHI__DSP__IMPULSE_RESPONSE_H__INCLUDED
#define HARUHI__DSP__IMPULSE_RESPONSE_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

namespace DSP {

enum ImpulseResponseType { FIR, IIR };


/**
 * Base class for impulse responses
 * that can be asked for… the response (magnitude).
 */
class PlotableImpulseResponse
{
  public:
	/**
	 * Computes response magnitude for given frequency (0, 0.5).
	 */
	virtual Sample
	response (Sample frequency) const = 0;
};


template<unsigned int tOrder, int tResponseType>
	class ImpulseResponse: public PlotableImpulseResponse
	{
	  public:
		typedef unsigned int Serial;

		enum {
			Order			= tOrder,
			ResponseType	= tResponseType,
		};

	  public:
		ImpulseResponse();

		virtual ~ImpulseResponse() { }

		/**
		 * Returns current serial number.
		 * Serial number changes whenever impulse response
		 * coefficients change.
		 */
		Serial
		serial() const;

	  protected:
		/**
		 * Increases serial number, so filter will
		 * know to reset recursive coefficients.
		 */
		void
		bump();

	  public:
		// Array of H(z)'s denominator coefficients. Unused in FIR filters.
		Sample	a[Order];
		// Array of H(z)'s numerator coefficients.
		Sample	b[Order];

	  private:
		Serial	_serial;
	};


template<unsigned int O, int T>
	inline
	ImpulseResponse<O, T>::ImpulseResponse():
		_serial (0)
	{ }


template<unsigned int O, int T>
	inline
	typename ImpulseResponse<O, T>::Serial
	ImpulseResponse<O, T>::serial() const
	{
		return _serial;
	}


template<unsigned int O, int T>
	inline void
	ImpulseResponse<O, T>::bump()
	{
		++_serial;
	}

} // namespace DSP

} // namespace Haruhi

#endif

