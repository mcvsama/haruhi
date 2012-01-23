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
	_source (vector),
	_target (vector)
{
	FFT::_plan_mutex.synchronize ([&]() noexcept {
		_plan = fftw_plan_dft_1d (vector.size(),
								  reinterpret_cast<fftw_complex*> (vector.data()),
								  reinterpret_cast<fftw_complex*> (vector.data()),
								  FFTW_FORWARD, FFTW_ESTIMATE);
	});
}


FFT::Forward::Forward (Vector& source, Vector& target) noexcept:
	_source (source),
	_target (target)
{
	assert (source.size() == target.size());

	FFT::_plan_mutex.synchronize ([&]() noexcept {
		_plan = fftw_plan_dft_1d (source.size(),
								  reinterpret_cast<fftw_complex*> (source.data()),
								  reinterpret_cast<fftw_complex*> (target.data()),
								  FFTW_FORWARD, FFTW_ESTIMATE);
	});
}


FFT::Forward::~Forward() noexcept
{
	FFT::_plan_mutex.synchronize ([&]() noexcept {
		fftw_destroy_plan (_plan);
	});
}


void
FFT::Forward::transform() noexcept
{
	fftw_execute (_plan);
}


FFT::Inverse::Inverse (Vector& vector) noexcept:
	_source (vector),
	_target (vector)
{
	FFT::_plan_mutex.synchronize ([&]() noexcept {
		_plan = fftw_plan_dft_1d (vector.size(),
								  reinterpret_cast<fftw_complex*> (vector.data()),
								  reinterpret_cast<fftw_complex*> (vector.data()),
								  FFTW_BACKWARD, FFTW_ESTIMATE);
	});
}


FFT::Inverse::Inverse (Vector& source, Vector& target) noexcept:
	_source (source),
	_target (target)
{
	assert (source.size() == target.size());

	FFT::_plan_mutex.synchronize ([&]() noexcept {
		_plan = fftw_plan_dft_1d (source.size(),
								  reinterpret_cast<fftw_complex*> (source.data()),
								  reinterpret_cast<fftw_complex*> (target.data()),
								  FFTW_BACKWARD, FFTW_ESTIMATE);
	});
}


FFT::Inverse::~Inverse() noexcept
{
	FFT::_plan_mutex.synchronize ([&]() noexcept {
		fftw_destroy_plan (_plan);
	});
}


void
FFT::Inverse::transform() noexcept
{
	fftw_execute (_plan);
	_target.normalize();
}

} // namespace DSP

} // namespace Haruhi

