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

#ifndef HARUHI__PLUGINS__YUKI__HAS_WIDGET_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__HAS_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>


namespace Yuki {

template<class WidgetClass>
	class HasWidget
	{
	  public:
		HasWidget() noexcept;

		/**
		 * Return associated UI object.
		 */
		WidgetClass*
		widget() const noexcept;

		/**
		 * Set associated UI object.
		 */
		void
		set_widget (WidgetClass* widget) noexcept;

	  private:
		WidgetClass* _widget;
	};


template<class WidgetClass>
	inline
	HasWidget<WidgetClass>::HasWidget() noexcept:
		_widget (0)
	{ }


template<class WidgetClass>
	inline WidgetClass*
	HasWidget<WidgetClass>::widget() const noexcept
	{
		return _widget;
	}


template<class WidgetClass>
	inline void
	HasWidget<WidgetClass>::set_widget (WidgetClass* widget) noexcept
	{
		_widget = widget;
	}

} // namespace Yuki

#endif

