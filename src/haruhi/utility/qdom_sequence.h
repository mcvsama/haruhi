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

#ifndef HARUHI__UTILITY__QDOM_SEQUENCE_H__INCLUDED
#define HARUHI__UTILITY__QDOM_SEQUENCE_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>

// Qt:
#include <QtXml/QDomElement>


namespace Haruhi {

/**
 * Sequence iterator for use with for(:) loops.
 */
class QDomElementIterator
{
  public:
	/**
	 * Create past-the-end iterator.
	 */
	QDomElementIterator() = default;

	QDomElementIterator (QDomElement element);

	bool
	operator== (QDomElementIterator const& other);

	bool
	operator!= (QDomElementIterator const& other);

	void
	operator++();

	QDomElement&
	operator*();

  private:
	QDomElement _element;
};


inline
QDomElementIterator::QDomElementIterator (QDomElement element):
	_element (element)
{ }


inline bool
QDomElementIterator::operator== (QDomElementIterator const& other)
{
	return _element == other._element;
}


inline bool
QDomElementIterator::operator!= (QDomElementIterator const& other)
{
	return _element != other._element;
}


inline void
QDomElementIterator::operator++()
{
	_element = _element.nextSiblingElement();
}


inline QDomElement&
QDomElementIterator::operator*()
{
	return _element;
}

} // namespace Haruhi


namespace std {

inline Haruhi::QDomElementIterator
begin (QDomElement element)
{
	return Haruhi::QDomElementIterator (element.firstChildElement());
}


inline Haruhi::QDomElementIterator
end (QDomElement)
{
	return Haruhi::QDomElementIterator();
}

} // namespace std

#endif

