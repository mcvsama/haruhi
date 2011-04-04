/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>

// Local:
#include "../mikuru.h"
#include "delay.h"


namespace MikuruPrivate {

Delay::Delay (int id, Mikuru* mikuru, QWidget* parent):
	Effect (id, "delays", mikuru, "Delay", new Params::Effect(), parent),
	_mikuru (mikuru),
	_params (static_cast<Params::Effect*> (Effect::params()))
{
}


Delay::~Delay()
{
}


void
Delay::process (Haruhi::AudioBuffer* in1, Haruhi::AudioBuffer* in2, Haruhi::AudioBuffer* out1, Haruhi::AudioBuffer* out2)
{
}

} // namespace MikuruPrivate

