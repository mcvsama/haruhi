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

#ifndef HARUHI__DSP__FFT_H__INCLUDED
#define HARUHI__DSP__FFT_H__INCLUDED

// Standard:
#include <cstddef>
#include <complex>

// Lib:
#include <fftw3.h>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/noncopyable.h>


namespace Haruhi {

namespace DSP {

class FFT
{
  public:
	typedef std::complex<double> Complex;

	class Vector: public Noncopyable
	{
	  public:
		Vector (std::size_t samples);

		~Vector();

		Complex&
		operator[] (int i) noexcept;

		Complex const&
		operator[] (int i) const noexcept;

		Complex*
		data() const noexcept;

		std::size_t
		size() const noexcept;

		void
		normalize() noexcept;

	  private:
		std::size_t	_size;
		Complex*	_data;
	};

	class Forward
	{
	  public:
		Forward (Vector& source) noexcept;

		Forward (Vector& source, Vector& target) noexcept;

		~Forward() noexcept;

		void
		transform() noexcept;

	  private:
		fftw_plan	_plan;
		Vector&		_source;
		Vector&		_target;
	};

	class Inverse
	{
	  public:
		Inverse (Vector& source) noexcept;

		Inverse (Vector& source, Vector& target) noexcept;

		~Inverse() noexcept;

		void
		transform() noexcept;

	  private:
		fftw_plan	_plan;
		Vector&		_source;
		Vector&		_target;
	};

  public:
	virtual void
	transform() = 0;

  private:
	// Creating/destroying plans is not thread safe in FFTW, mutex is needed:
	static Mutex _plan_mutex;
};


inline FFT::Complex&
FFT::Vector::operator[] (int i) noexcept
{
	return _data[i];
}


inline FFT::Complex const&
FFT::Vector::operator[] (int i) const noexcept
{
	return _data[i];
}


inline FFT::Complex*
FFT::Vector::data() const noexcept
{
	return _data;
}


inline std::size_t
FFT::Vector::size() const noexcept
{
	return _size;
}

} // namespace DSP

} // namespace Haruhi

#endif

