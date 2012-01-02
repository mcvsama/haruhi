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

#ifndef HARUHI__DSP__FILTER_H__INCLUDED
#define HARUHI__DSP__FILTER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/impulse_response.h>
#include <haruhi/utility/exception.h>


namespace Haruhi {

namespace DSP {

/*
 * Implementation of IIR filters described in
 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 */
template<unsigned int tOrder, int tResponseType>
	class Filter
	{
	  public:
		static const unsigned int Order = tOrder;
		static const int ResponseType = tResponseType;

	  private:
		typedef ImpulseResponse<Order, ResponseType> ImpulseResponseType;

	  public:
		/**
		 * Create filter with no impulse response.
		 */
		Filter();

		/**
		 * Create filter with given impulse response.
		 * Filter does not take ownership of impulse_response.
		 */
		Filter (ImpulseResponseType* impulse_response);

		/**
		 * Resets the filter to state as it is newly created.
		 */
		void
		reset();

		void
		assign_impulse_response (ImpulseResponseType* impulse_response);

		/**
		 * Note: input and output sequences must be distinct!
		 * InputIterator and OutputIterator must implement concept of RandomAccessIterator.
		 */
		template<class InputIterator, class OutputIterator>
			void
			transform (InputIterator begin, InputIterator end, OutputIterator output);

	  private:
		Sample
		advance_fir (Sample* x);

		Sample
		advance_iir (Sample* x, Sample* y);

		Sample
		mixed_advance_fir (Sample* x, int position);

		Sample
		mixed_advance_iir (Sample* x, Sample* y, int position);

	  private:
		ImpulseResponseType*					_impulse_response;
		typename ImpulseResponseType::Serial	_last_serial;
		// Previous samples buffer, stored in reverse order (index 0 contains last sample, 1 one before last, etc):
		Sample	_px[Order];
		Sample	_py[Order];
	};


template<unsigned int O, int R>
	inline
	Filter<O, R>::Filter():
		_impulse_response (0),
		_last_serial (0)
	{ }


template<unsigned int O, int R>
	inline
	Filter<O, R>::Filter (ImpulseResponseType* impulse_response):
		_impulse_response (0),
		_last_serial (0)
	{
		assign_impulse_response (impulse_response);
	}


template<unsigned int O, int R>
	inline void
	Filter<O, R>::reset()
	{
		std::fill (_px, _px + Order, 0.0f);
		std::fill (_py, _py + Order, 0.0f);
	}


template<unsigned int O, int R>
	inline void
	Filter<O, R>::assign_impulse_response (ImpulseResponseType* impulse_response)
	{
		_impulse_response = impulse_response;

		std::fill (_px, _px + Order, 0.0f);
		std::fill (_py, _py + Order, 0.0f);
	}


template<unsigned int O, int R>
	template<class InputIterator, class OutputIterator>
		inline void
		Filter<O, R>::transform (InputIterator begin, InputIterator end, OutputIterator output)
		{
			if (std::distance (begin, end) < static_cast<int> (Order))
			{
				for (InputIterator k = begin; k != end; ++k)
					*output++ = 0.0f;
				return;
			}

			// Protect from denormals by adding a constant.
			// This will propagate to further stages:
			for (InputIterator c = begin; c != end; ++c)
				*c += 1e-30;

			std::copy (begin, end, output);

			if (_impulse_response)
			{
				InputIterator current = begin;

				// If this is IIR filter:
				if (ResponseType == static_cast<int> (IIR))
				{
					// Filter first part using previous-buffers:
					for (int i = 0; i < static_cast<int> (Order) - 1; ++i, ++current, ++output)
						*output = mixed_advance_iir (current, output, i);

					// Filter the rest:
					for (; current != end; ++current, ++output)
						*output = advance_iir (current, output);

					// Store last samples as previous-buffer for next round
					// (current and output are now after the end of sequences):
					for (int i = 0; i < static_cast<int> (Order) - 1; ++i)
					{
						_px[i] = current[-i-1];
						_py[i] = output[-i-1];
					}
				}
				// If this is FIR filter:
				else
				{
					// Filter first part using previous-buffers:
					for (int i = 0; i < static_cast<int> (Order) - 1; ++i, ++current, ++output)
						*output = mixed_advance_fir (current, i);

					// Filter the rest:
					for (; current != end; ++current, ++output)
						*output = advance_fir (current);

					// Store last samples as previous-buffer for next round
					// (current and output are now after the end of sequences):
					for (int i = 0; i < static_cast<int> (Order) - 1; ++i)
						_px[i] = current[-i-1];
				}
			}
		}


template<unsigned int O, int R>
	inline Sample
	Filter<O, R>::advance_fir (Sample* x)
	{
		Sample (&b)[Order] = _impulse_response->b;
		Sample register sum = 0.0;
		for (int i = 0; i < static_cast<int> (Order); ++i)
			sum += b[i] * x[-i];
		return sum;
	}


template<unsigned int O, int R>
	inline Sample
	Filter<O, R>::advance_iir (Sample* x, Sample* y)
	{
		Sample (&a)[Order] = _impulse_response->a;
		Sample (&b)[Order] = _impulse_response->b;
		Sample register sum = 0.0;
		for (int i = 0; i < static_cast<int> (Order); ++i)
			sum += b[i] * x[-i] - a[i] * y[-i];
		return sum;
	}


template<unsigned int O, int R>
	inline Sample
	Filter<O, R>::mixed_advance_fir (Sample* x, int position)
	{
		Sample (&b)[Order] = _impulse_response->b;
		Sample register sum = 0.0;
		for (int i = 0; i < static_cast<int> (Order); ++i)
			sum += b[i] * (i > position ? _px[i-1-position] : x[-i]);
		return sum;
	}


template<unsigned int O, int R>
	inline Sample
	Filter<O, R>::mixed_advance_iir (Sample* x, Sample* y, int position)
	{
		Sample (&a)[Order] = _impulse_response->a;
		Sample (&b)[Order] = _impulse_response->b;
		Sample register sum = 0.0;
		for (int i = 0; i < static_cast<int> (Order); ++i)
			sum += b[i] * (i > position ? _px[i-1-position] : x[-i]) - a[i] * (i > position ? _py[i-1-position] : y[-i]);
		return sum;
	}

} // namespace DSP

} // namespace Haruhi

#endif

