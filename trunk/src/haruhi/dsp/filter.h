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

#ifndef HARUHI__DSP__FILTER_H__INCLUDED
#define HARUHI__DSP__FILTER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Haruhi:
#include <haruhi/core/audio.h>
#include <haruhi/dsp/impulse_response.h>
#include <haruhi/exception.h>


namespace DSP {

/*
 * Implementation of IIR filters described in
 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 */
class Filter
{
  public:
	/**
	 * Creates filter with no impulse response.
	 */
	Filter();

	/**
	 * Creates filter with given impulse response.
	 * Filter does not take ownership of impulse_response.
	 */
	Filter (ImpulseResponse* impulse_response);

	~Filter();

	/**
	 * Resets the filter to state as it is newly created.
	 */
	void
	reset();

	void
	assign_impulse_response (ImpulseResponse* impulse_response);

	/**
	 * Note: input and output sequences must be distinct!
	 * InputIterator and OutputIterator must implement concept of RandomAccessIterator.
	 */
	template<class InputIterator, class OutputIterator>
		void
		transform (InputIterator begin, InputIterator end, OutputIterator output)
		{
			if (std::distance (begin, end) < _size)
			{
				*output++ = 0.0f;
				return;
			}

			std::copy (begin, end, output);

			if (_impulse_response)
			{
				InputIterator current = begin;

				// If this is IIR filter:
				if (_a)
				{
					// Filter first part using previous-buffers:
					for (int i = 0; i < _size - 1; ++i, ++current, ++output)
						*output = mixed_advance_iir (current, output, i);

					// Filter the rest:
					for (; current != end; ++current, ++output)
						*output = advance_iir (current, output);

					// Store last samples as previous-buffer for next round
					// (current and output are now after the end of sequences):
					for (int i = 0; i < _size - 1; ++i)
					{
						_px[i] = current[-i-1];
						_py[i] = output[-i-1];
					}
				}
				// If this is FIR filter:
				else
				{
					// Filter first part using previous-buffers:
					for (int i = 0; i < _size - 1; ++i, ++current, ++output)
						*output = mixed_advance_fir (current, i);

					// Filter the rest:
					for (; current != end; ++current, ++output)
						*output = advance_fir (current);

					// Store last samples as previous-buffer for next round
					// (current and output are now after the end of sequences):
					for (int i = 0; i < _size - 1; ++i)
						_px[i] = current[-i-1];
				}
			}
		}

  private:
	Core::Sample
	advance_fir (Core::Sample* x)
	{
		Core::Sample register sum = 0.0;
		for (int i = 0; i < _size; ++i)
			sum += _b[i] * x[-i];
		return sum;
	}

	Core::Sample
	advance_iir (Core::Sample* x, Core::Sample* y)
	{
		Core::Sample register sum = 0.0;
		for (int i = 0; i < _size; ++i)
			sum += _b[i] * x[-i] - _a[i] * y[-i];
		return sum;
	}

	Core::Sample
	mixed_advance_fir (Core::Sample* x, int position)
	{
		Core::Sample register sum = 0.0;
		for (int i = 0; i < _size; ++i)
			sum += _b[i] * (i > position ? _px[i-1-position] : x[-i]);
		return sum;
	}

	Core::Sample
	mixed_advance_iir (Core::Sample* x, Core::Sample* y, int position)
	{
		Core::Sample register sum = 0.0;
		for (int i = 0; i < _size; ++i)
			sum += _b[i] * (i > position ? _px[i-1-position] : x[-i]) - _a[i] * (i > position ? _py[i-1-position] : y[-i]);
		return sum;
	}

  private:
	ImpulseResponse*		_impulse_response;
	ImpulseResponse::Serial	_last_serial;
	// Cached data from ImpulseResponse:
	Core::Sample*			_a;
	Core::Sample*			_b;
	int						_size;
	// Previous samples buffer, stored in reverse order (index 0 contains last sample, 1 one before last, etc):
	Core::Sample*			_px;
	Core::Sample*			_py;
};

} // namespace DSP

#endif

