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

#ifndef HARUHI__GRAPH__BUFFER_H__INCLUDED
#define HARUHI__GRAPH__BUFFER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/noncopyable.h>


namespace Haruhi {

/**
 * Port's buffer (for audio, events).
 *
 * For performance reasons dynamic_casts should not be used.
 * Instead use type() method to identify type. It should return unique pointer
 * to type-name string. Every subclass should define constant like eg.
 * EventBuffer::TYPE to the string.
 */
class Buffer: public Noncopyable
{
  public:
	typedef const char* TypeID;

  public:
	Buffer (TypeID type);

	virtual ~Buffer() { }

	/**
	 * Returns string (pointer) that identifies
	 * type. Comparison is done using pointers, not string contents.
	 */
	TypeID
	type() const;

	/**
	 * Clears buffer.
	 */
	virtual void
	clear() = 0;

	/**
	 * Mixes in another buffer.
	 */
	virtual void
	mixin (Buffer const*) = 0;

  private:
	TypeID _type;
};


inline
Buffer::Buffer (TypeID type):
	_type (type)
{ }


inline Buffer::TypeID
Buffer::type() const
{
	return _type;
}

} // namespace Haruhi

#endif

