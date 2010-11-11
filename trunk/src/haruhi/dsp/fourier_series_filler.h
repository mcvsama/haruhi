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

#ifndef HARUHI__DSP__FOURIER_SERIES_FILLER_H__INCLUDED
#define HARUHI__DSP__FOURIER_SERIES_FILLER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/dsp/wavetable.h>


namespace Haruhi {

namespace DSP {

class FourierSeriesFiller: public Wavetable::Filler
{
  public:
	typedef Sample (*Function)(Sample, Sample, Sample, Sample);

  public:
	FourierSeriesFiller (Function function);

	/**
	 * \param	tables
	 * 			Tables vector to be created.
	 * \param	number
	 * 			Desired number of tables to be generated.
	 * \param	samples
	 * 			Desired number of samples for each table.
	 */
	void
	fill (Wavetable::Tables& tables, int number, int samples);

  private:
	Function _function;
};

} // namespace DSP

} // namespace Haruhi

#endif

