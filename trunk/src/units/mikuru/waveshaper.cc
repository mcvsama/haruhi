/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
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

// Qt:
#include <QtGui/QToolTip>
#include <Qt3Support/Q3GroupBox>

// Local:
#include "mikuru.h"
#include "part.h"
#include "waveshaper.h"
#include "params.h"


namespace MikuruPrivate {

Waveshaper::Waveshaper (Part* part, Haruhi::PortGroup* port_group, QString const& port_prefix, Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_part (part),
	_loading_params (false)
{
//	_control = new Knob (this, "Param", 1000, 10000, 1.0, 10.0, 1, 2, 1000);
//	_control->assign_parameter (_i);
//	_control->assign_unit_bay (_mikuru->unit_bay());
}


Waveshaper::~Waveshaper()
{
}


float sh (float input)
{
	const float x = input * 0.686306;
	const float a = 1 + expf (sqrtf (fabsf (x)) * -0.75);
	return (expf (x) - expf (-x * a)) / (expf (x) + expf (-x));

//	return 1.5f * input - 0.5f * input * input * input;
}


void
Waveshaper::process (Haruhi::AudioBuffer* buffer1, Haruhi::AudioBuffer* buffer2)
{
	// TODO temporary hardcoded polynomial waveshaper:
	for (Sample* s = buffer1->begin(); s != buffer1->end(); ++s)
		*s = sh (*s);
	for (Sample* s = buffer2->begin(); s != buffer2->end(); ++s)
		*s = sh (*s);
}


void
Waveshaper::load_params()
{
}


void
Waveshaper::update_params()
{
}


void
Waveshaper::update_widgets()
{
}

} // namespace MikuruPrivate

