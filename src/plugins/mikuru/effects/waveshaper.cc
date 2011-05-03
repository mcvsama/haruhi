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
#include <cmath>
#include <algorithm>

// Qt:
#include <QtGui/QLayout>
#include <QtGui/QGroupBox>

// Haruhi:
#include <haruhi/utility/memory.h>

// Local:
#include "../mikuru.h"
#include "../params.h"
#include "waveshaper.h"


namespace MikuruPrivate {

namespace Waveshapers {

	/*
	 * Gloubi-boulga waveshaper
	 * <http://musicdsp.org/showArchiveComment.php?ArchiveID=86>
	 */

	inline Sample
	gloubi_boulga (Sample x, Sample)
	{
		x *= 0.686306f;
		const float a = 1.0f + expf (sqrtf (fabsf (x)) * -0.75f);
		return (expf (x) - expf (-x * a)) / (expf (x) + expf (-x));
	}

	/*
	 * Simpler implementation of Gloubi-boulga waveshaper
	 * x - 0.15 * x^2 - 0.15 * x^3
	 */

	inline Sample
	gloubi_boulga_simple (Sample x, Sample)
	{
		x = bound (x, -1.0f, +1.0f);
		return x - 0.15f * x * x - 0.15f * x * x * x;
	}

	/*
	 * Simple cubic waveshaper
	 * <http://musicdsp.org/showArchiveComment.php?ArchiveID=114>
	 */

	inline Sample
	warmth (Sample x, Sample)
	{
		x = bound (x, -1.0f, +1.0f);
		return 1.5f * x - 0.5f * x * x * x;
	}

	/*
	 * Saturation waveshaper
	 * <http://musicdsp.org/showArchiveComment.php?ArchiveID=43>
	 */

	inline Sample
	saturation_1 (Sample x, Sample k)
	{
		float z = M_PI * k * 0.9 + 0.01;
		float s = 1.f / sin (z);
		float b = 1.f / k;

		if (x > b)
		  return 1.f;
		else
		  return sin (z * x) * s;
	}

} // namespace Waveshapers


Waveshaper::Waveshaper (int id, Mikuru* mikuru, QWidget* parent):
	Effect (id, "waveshapers", mikuru, "Waveshaper", new Params::Waveshaper(), parent),
	_mikuru (mikuru),
	_params (static_cast<Params::Waveshaper*> (Effect::params())),
	_loading_params (false)
{
	_shapers.push_back (new Shaper (Waveshapers::gloubi_boulga, false));
	_shapers.push_back (new Shaper (Waveshapers::gloubi_boulga_simple, false));
	_shapers.push_back (new Shaper (Waveshapers::warmth, false));
	_shapers.push_back (new Shaper (Waveshapers::saturation_1, true));

	QGroupBox* box = new QGroupBox (this);
	box->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	QWidget* label_and_combo = new QWidget (this);
	label_and_combo->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Maximum);

	QLabel* type_label = new QLabel ("Waveshaper type:", label_and_combo);

	_waveshaper_type_combo = new QComboBox (label_and_combo);
	_waveshaper_type_combo->addItem ("Gloubi-boulga", QVariant::fromValue (_shapers[0]));
	_waveshaper_type_combo->addItem ("Gloubi-boulga simple (fast)", QVariant::fromValue (_shapers[1]));
	_waveshaper_type_combo->addItem ("Warmth", QVariant::fromValue (_shapers[2]));
	_waveshaper_type_combo->addItem ("Saturation", QVariant::fromValue (_shapers[3]));
	QObject::connect (_waveshaper_type_combo, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_waveshaper_type_combo, SIGNAL (activated (int)), this, SLOT (update_widgets()));

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	_port_gain = new Haruhi::EventPort (_mikuru, "Gain", Haruhi::Port::Input, port_group());
	_port_parameter = new Haruhi::EventPort (_mikuru, "Parameter", Haruhi::Port::Input, port_group());
	if (_mikuru->graph())
		_mikuru->graph()->unlock();

	_knob_gain = new Haruhi::Knob (this, _port_gain, &_params->gain, "Gain", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Waveshaper::Gain, 100), 2);
	_knob_parameter = new Haruhi::Knob (this, _port_parameter, &_params->parameter, "Parameter", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Waveshaper::Parameter, 100), 2);

	_knob_gain->set_unit_bay (_mikuru->unit_bay());
	_knob_parameter->set_unit_bay (_mikuru->unit_bay());

	QHBoxLayout* label_and_combo_layout = new QHBoxLayout (label_and_combo);
	label_and_combo_layout->setMargin (0);
	label_and_combo_layout->setSpacing (Config::Spacing);
	label_and_combo_layout->addWidget (type_label);
	label_and_combo_layout->addWidget (_waveshaper_type_combo);

	QHBoxLayout* box_layout = new QHBoxLayout (box);
	box_layout->setMargin (0);
	box_layout->setSpacing (Config::Spacing);
	box_layout->addWidget (label_and_combo);

	QHBoxLayout* layout = new QHBoxLayout (parent_widget());
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (box);
	layout->addWidget (_knob_gain);
	layout->addWidget (_knob_parameter);

	update_params();
	update_widgets();
}


Waveshaper::~Waveshaper()
{
	delete _knob_gain;
	delete _knob_parameter;

	std::for_each (_shapers.begin(), _shapers.end(), delete_operator<Shaper*>);

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	delete _port_gain;
	delete _port_parameter;
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
Waveshaper::process_events()
{
	Effect::process_events();

	_knob_gain->process_events();
	_knob_parameter->process_events();
}


void
Waveshaper::process (Haruhi::AudioBuffer* in1, Haruhi::AudioBuffer* in2, Haruhi::AudioBuffer* out1, Haruhi::AudioBuffer* out2)
{
	Shaper::Function f = current_shaper()->function;
	float gain = _params->gain.to_f();
	float parameter = _params->parameter.to_f();

	Sample* in;
	Sample* out;

	for (in = in1->begin(), out = out1->begin(); in != in1->end(); ++in, ++out)
		*out = (*f) (gain * *in, parameter);
	for (in = in2->begin(), out = out2->begin(); in != in2->end(); ++in, ++out)
		*out = (*f) (gain * *in, parameter);
}


void
Waveshaper::panic()
{
}


void
Waveshaper::load_params()
{
	// Copy params:
	Params::Waveshaper p (*_params);
	_loading_params = true;

	Effect::load_params();

	_waveshaper_type_combo->setCurrentIndex (p.type);

	_loading_params = false;
	update_widgets();
}


void
Waveshaper::load_params (Params::Waveshaper& params)
{
	*_params = params;
	load_params();
}


void
Waveshaper::update_params()
{
	if (_loading_params)
		return;

	Effect::update_params();

	_params->type.set (_waveshaper_type_combo->currentIndex());

	// Knob params are updated automatically using #assign_parameter.

	_params->sanitize();
}


void
Waveshaper::update_widgets()
{
	_knob_parameter->setEnabled (current_shaper()->has_parameter);
}


Shaper*
Waveshaper::current_shaper()
{
	return _waveshaper_type_combo->itemData (_params->type.get()).value<Shaper*>();
}

} // namespace MikuruPrivate

