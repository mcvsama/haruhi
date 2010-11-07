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
#include <haruhi/config/system.h>
#include <haruhi/exception.h>
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
FFT::Vector::normalize()
{
	for (std::size_t i = 0; i < _size; ++i)
		_data[i] /= _size;
}


FFT::Forward::Forward (Vector& vector):
	_source (vector),
	_target (vector)
{
	FFT::_plan_mutex.lock();
	_plan = fftw_plan_dft_1d (vector.size(),
							  reinterpret_cast<fftw_complex*> (vector.data()),
							  reinterpret_cast<fftw_complex*> (vector.data()),
							  FFTW_FORWARD, FFTW_ESTIMATE);
	FFT::_plan_mutex.unlock();
}


FFT::Forward::Forward (Vector& source, Vector& target):
	_source (source),
	_target (target)
{
	assert (source.size() == target.size());

	FFT::_plan_mutex.lock();
	_plan = fftw_plan_dft_1d (source.size(),
							  reinterpret_cast<fftw_complex*> (source.data()),
							  reinterpret_cast<fftw_complex*> (target.data()),
							  FFTW_FORWARD, FFTW_ESTIMATE);
	FFT::_plan_mutex.unlock();
}


FFT::Forward::~Forward()
{
	FFT::_plan_mutex.lock();
	fftw_destroy_plan (_plan);
	FFT::_plan_mutex.unlock();
}


void
FFT::Forward::transform()
{
	fftw_execute (_plan);
}


FFT::Inverse::Inverse (Vector& vector):
	_source (vector),
	_target (vector)
{
	FFT::_plan_mutex.lock();
	_plan = fftw_plan_dft_1d (vector.size(),
							  reinterpret_cast<fftw_complex*> (vector.data()),
							  reinterpret_cast<fftw_complex*> (vector.data()),
							  FFTW_BACKWARD, FFTW_ESTIMATE);
	FFT::_plan_mutex.unlock();
}


FFT::Inverse::Inverse (Vector& source, Vector& target):
	_source (source),
	_target (target)
{
	assert (source.size() == target.size());

	FFT::_plan_mutex.lock();
	_plan = fftw_plan_dft_1d (source.size(),
							  reinterpret_cast<fftw_complex*> (source.data()),
							  reinterpret_cast<fftw_complex*> (target.data()),
							  FFTW_BACKWARD, FFTW_ESTIMATE);
	FFT::_plan_mutex.unlock();
}


FFT::Inverse::~Inverse()
{
	FFT::_plan_mutex.lock();
	fftw_destroy_plan (_plan);
	FFT::_plan_mutex.unlock();
}


void
FFT::Inverse::transform()
{
	fftw_execute (_plan);
	_target.normalize();
}

} // namespace DSP

} // namespace Haruhi

