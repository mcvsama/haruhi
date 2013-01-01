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

#ifndef HARUHI__PLUGINS__EG__PARAMS_H__INCLUDED
#define HARUHI__PLUGINS__EG__PARAMS_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/lib/controller_param.h>
#include <haruhi/lib/param.h>
#include <haruhi/plugin/saveable_params.h>


namespace EG {

struct Params
{
	/**
	 * ADSR params.
	 */
	struct ADSR: public Haruhi::SaveableParams<ADSR>
	{
		HARUHI_SAVEABLE_PARAMS_STANDARD_METHODS (ADSR)

		Haruhi::Param<int> enabled;
		Haruhi::Param<int> bypassed;

		static const std::size_t NUM_PARAMS = 2;
	};
};

} // namespace EG

#endif

