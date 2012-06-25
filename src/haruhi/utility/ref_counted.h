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

#ifndef HARUHI__UTILITY__REF_COUNTED_H__INCLUDED
#define HARUHI__UTILITY__REF_COUNTED_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/noncopyable.h>


/**
 * Implements reference-counted object. Initial reference count is 1.
 * When this number hits 0, it will be deleted. This means that objects
 * of this type MUST be allocated on heap (using 'new' operator).
 * Explicit deletion of this object is permitted.
 */
class RefCounted: private Noncopyable
{
  public:
	constexpr
	RefCounted() noexcept:
		_ref_count (1)
	{ }

	/**
	 * Increases references number.
	 */
	void
	ref() noexcept
	{
		_ref_count += 1;
	}

	/**
	 * Decreases references number. When number hits 0, object deletes itself
	 * using 'delete' operator. So it is not safe to use object after unref() operation.
	 */
	void
	unref() noexcept
	{
		if (--_ref_count == 0)
			delete this;
	}

	/**
	 * Returns number of references to this object.
	 */
	unsigned int
	ref_num() const noexcept
	{
		return _ref_count;
	}

  private:
	unsigned int _ref_count;
};

#endif

