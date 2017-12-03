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

#ifndef HARUHI__LIB__UNIT__MONO_PARAM_GOVERNOR_H__INCLUDED
#define HARUHI__LIB__UNIT__MONO_PARAM_GOVERNOR_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/lib/unit/param_governor.h>


namespace Haruhi {

/**
 * Specialized version for parameters in monophonic operation
 * (one voice in a monophonic instrument or a parameter of an effect).
 */
template<class pParamValue>
	class MonoParamGovernor: public ParamGovernor<pParamValue>
	{
	  public:
		ParamValue*	_param;
	};

} // namespace Haruhi

#endif

