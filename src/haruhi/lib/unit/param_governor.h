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

#ifndef HARUHI__LIB__UNIT__PARAM_GOVERNOR_H__INCLUDED
#define HARUHI__LIB__UNIT__PARAM_GOVERNOR_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

/**
 * Governs interactions between a sound parameter, an input EventPort for controlling that parameter and user interface
 * widget representing the parameter. Updates Param according to both events coming from EventPorts and from
 * UIControllerWidget.
 */
template<class pParamValue>
	class ParamGovernor
	{
	  public:
		using ParamValue = pParamValue;

	  public:
		// Ctor
		explicit
		ParamGovernor (EventPort* main_input, EventPort* modulation_input, UIControllerWidget* ui_controller);

	  private:
		EventPort*			_main_input;
		EventPort*			_modulation_input;
		float				_modulation_intensity; // TODO?
		UIControllerWidget*	_widget;
	};

} // namespace Haruhi

#endif

