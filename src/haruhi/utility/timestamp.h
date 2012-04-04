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

#ifndef HARUHI__UTILITY__TIMESTAMP_H__INCLUDED
#define HARUHI__UTILITY__TIMESTAMP_H__INCLUDED

// Standard:
#include <cstddef>

// System:
#include <sys/time.h>

// Haruhi:
#include <haruhi/config/all.h>


class Timestamp
{
  public:
	/**
	 * Construct "0" timestamp.
	 */
	Timestamp() noexcept;

	bool
	operator< (Timestamp const& other) const noexcept;

	bool
	operator> (Timestamp const& other) const noexcept;

	Timestamp
	operator-() const noexcept;

	Timestamp&
	operator+= (Timestamp const& other) noexcept;

	Timestamp&
	operator-= (Timestamp const& other) noexcept;

	Timestamp
	operator+ (Timestamp const& other) const noexcept;

	Timestamp
	operator- (Timestamp const& other) const noexcept;

	/**
	 * Return UNIX time for given timestamp measured in µs.
	 */
	int64_t
	microseconds() const noexcept;

	/**
	 * Assign current time to the timestamp.
	 */
	void
	touch() noexcept;

	/**
	 * Assign given value of Epoch seconds to the timestamp.
	 */
	void
	set_epoch (int64_t epoch) noexcept;

	/**
	 * Assign given value of Epoch microseconds to the timestamp.
	 */
	void
	set_epoch_microseconds (int64_t epoch_microseconds) noexcept;

  public:
	static Timestamp
	now() noexcept;

	static Timestamp
	from_epoch (int64_t epoch) noexcept;

	static Timestamp
	from_epoch_microseconds (int64_t epoch_us) noexcept;

  private:
	// Ctor
	explicit Timestamp (int64_t epoch_microseconds) noexcept;

  private:
	int64_t _epoch_us;
};


inline
Timestamp::Timestamp() noexcept:
	_epoch_us (0)
{ }


inline bool
Timestamp::operator< (Timestamp const& other) const noexcept
{
	return _epoch_us < other._epoch_us;
}


inline bool
Timestamp::operator> (Timestamp const& other) const noexcept
{
	return other < *this;
}


inline Timestamp
Timestamp::operator-() const noexcept
{
	return Timestamp (-_epoch_us);
}


inline Timestamp&
Timestamp::operator+= (Timestamp const& other) noexcept
{
	_epoch_us += other._epoch_us;
	return *this;
}


inline Timestamp&
Timestamp::operator-= (Timestamp const& other) noexcept
{
	_epoch_us -= other._epoch_us;
	return *this;
}


inline Timestamp
Timestamp::operator+ (Timestamp const& other) const noexcept
{
	return Timestamp (_epoch_us + other._epoch_us);
}


inline Timestamp
Timestamp::operator- (Timestamp const& other) const noexcept
{
	return Timestamp (_epoch_us - other._epoch_us);
}


inline int64_t
Timestamp::microseconds() const noexcept
{
	return _epoch_us;
}


inline void
Timestamp::touch() noexcept
{
	struct timeval t;
	::gettimeofday (&t, 0);
	_epoch_us = t.tv_sec * 1000000ull + t.tv_usec;
}


inline void
Timestamp::set_epoch (int64_t epoch) noexcept
{
	_epoch_us = epoch * 1000000ull;
}


inline void
Timestamp::set_epoch_microseconds (int64_t epoch_microseconds) noexcept
{
	_epoch_us = epoch_microseconds;
}


inline Timestamp
Timestamp::now() noexcept
{
	Timestamp t;
	t.touch();
	return t;
}


inline Timestamp
Timestamp::from_epoch (int64_t epoch) noexcept
{
	return Timestamp (epoch * 1000000ull);
}


inline Timestamp
Timestamp::from_epoch_microseconds (int64_t epoch_us) noexcept
{
	return Timestamp {epoch_us};
}


inline
Timestamp::Timestamp (int64_t epoch_us) noexcept:
	_epoch_us (epoch_us)
{ }

#endif

