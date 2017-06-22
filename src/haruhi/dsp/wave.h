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

#ifndef HARUHI__DSP__WAVE_H__INCLUDED
#define HARUHI__DSP__WAVE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>
#include <utility>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/noncopyable.h>


namespace Haruhi {

namespace DSP {

class Wave: private Noncopyable
{
  public:
	/**
	 * \param	immutable should be true if function does not change shape
	 *			between calls to operator() without changing wave's parameters.
	 *			Typically true, but noise functions will set this to false.
	 */
	Wave (bool immutable) noexcept;

	/**
	 * \param	wave Inner wave. This constructs decorator wave.
	 * \param	auto_delete Whether decorated wave should be deleted along with this one.
	 */
	Wave (Wave* inner_wave, bool auto_delete = false) noexcept;

	/**
	 * Dtor
	 */
	virtual ~Wave();

	/**
	 * Returns true if this wave or decorated wave is immutable.
	 */
	virtual bool
	immutable() const noexcept;

	/**
	 * Set immutable attribute for this wave or or decorated wave.
	 */
	virtual void
	set_immutable (bool immutable) noexcept;

	/**
	 * Return inner wave object.
	 * Returns 0 if this wave is not a decorator.
	 */
	Wave*
	inner_wave() const noexcept;

	/**
	 * Set new inner wave.
	 */
	void
	set_inner_wave (Wave* wave, bool auto_delete = false) noexcept;

	/**
	 * Returns function's sample.
	 * \param	phase is the phase in range [0, 1].
	 * \param	frequency is the base frequency of the signal this sample will
	 *			be used in (this is for limiting bandwidth). This is absolute
	 *			frequency (in range [0..1]). If you want to get highest possible
	 *			bandwidth, pass 0.0.
	 * \param	sample
	 *			Sample number for mutating waves. When generating a buffer of samples,
	 *			pass sample index as this argument.
	 */
	virtual Sample
	operator() (Sample phase, Sample frequency, std::size_t sample) const noexcept = 0;

	/**
	 * Compute average wave energy.
	 * \param	samples Use this many samples for computation. Lesser = faster.
	 */
	virtual Sample
	compute_average_energy (unsigned int samples = 1024) const noexcept;

	/**
	 * Compute min/max values of the wave.
	 * \param	samples User this many samples for computation. Lesser = faster.
	 */
	virtual std::pair<Sample, Sample>
	compute_min_max (unsigned int samples = 1024) const noexcept;

  private:
	bool	_immutable		= true;
	Wave*	_inner_wave		= nullptr;
	bool	_auto_delete	= false;
};


inline
Wave::Wave (bool immutable) noexcept:
	_immutable (immutable)
{ }


inline
Wave::Wave (Wave* inner_wave, bool auto_delete) noexcept:
	_inner_wave (inner_wave),
	_auto_delete (auto_delete)
{ }


inline
Wave::~Wave()
{
	if (_auto_delete)
		delete _inner_wave;
}


inline bool
Wave::immutable() const noexcept
{
	if (_inner_wave)
		return _inner_wave->immutable();
	else
		return _immutable;
}


inline void
Wave::set_immutable (bool immutable) noexcept
{
	if (_inner_wave)
		_inner_wave->set_immutable (immutable);
	else
		_immutable = immutable;
}


inline Wave*
Wave::inner_wave() const noexcept
{
	return _inner_wave;
}


inline void
Wave::set_inner_wave (Wave* wave, bool auto_delete) noexcept
{
	if (_auto_delete)
		delete _inner_wave;
	_auto_delete = auto_delete;
	_inner_wave = wave;
}

} // namespace DSP

} // namespace Haruhi

#endif

