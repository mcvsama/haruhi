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

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/system.h>
#include <haruhi/utility/exception.h>
#include <haruhi/utility/mutex.h>

// Local:
#include "fft.h"


namespace Haruhi {

namespace DSP {

Mutex FFT::_plan_mutex;


FFT::Vector::Vector (std::size_t samples):
	_size (samples),
	_data (static_cast<Complex*> (fftw_malloc (samples * sizeof (Complex))))
{
}


FFT::Vector::~Vector()
{
	fftw_free (_data);
}


void
FFT::Vector::normalize() noexcept
{
	for (std::size_t i = 0; i < _size; ++i)
		_data[i] /= _size;
}


FFT::Forward::Forward (Vector& vector) noexcept:
	Forward (vector, vector)
{ }


FFT::Forward::Forward (Vector& source, Vector& target) noexcept:
	_source (source),
	_target (target)
{
	assert (_source.size() == _target.size());

	Mutex::Lock lock (FFT::_plan_mutex);
	_plan = fftw_plan_dft_1d (_source.size(),
							  reinterpret_cast<fftw_complex*> (_source.data()),
							  reinterpret_cast<fftw_complex*> (_target.data()),
							  FFTW_FORWARD, FFTW_ESTIMATE);
}


FFT::Forward::~Forward() noexcept
{
	Mutex::Lock lock (FFT::_plan_mutex);
	fftw_destroy_plan (_plan);
}


void
FFT::Forward::transform() noexcept
{
	fftw_execute (_plan);
}


FFT::Inverse::Inverse (Vector& vector) noexcept:
	Inverse (vector, vector)
{ }


FFT::Inverse::Inverse (Vector& source, Vector& target) noexcept:
	_source (source),
	_target (target)
{
	assert (source.size() == target.size());

	Mutex::Lock lock (FFT::_plan_mutex);
	_plan = fftw_plan_dft_1d (source.size(),
							  reinterpret_cast<fftw_complex*> (_source.data()),
							  reinterpret_cast<fftw_complex*> (_target.data()),
							  FFTW_BACKWARD, FFTW_ESTIMATE);
}


FFT::Inverse::~Inverse() noexcept
{
	Mutex::Lock lock (FFT::_plan_mutex);
	fftw_destroy_plan (_plan);
}


void
FFT::Inverse::transform() noexcept
{
	fftw_execute (_plan);
	_target.normalize();
}

} // namespace DSP

} // namespace Haruhi

