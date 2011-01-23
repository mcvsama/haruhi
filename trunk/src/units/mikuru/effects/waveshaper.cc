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
	Effect (parent),
	_mikuru (mikuru),
	_loading_params (false)
{
	_id = (id == 0) ? _mikuru->allocate_id ("waveshapers") : _mikuru->reserve_id ("waveshapers", id);

	_shapers.push_back (new Shaper (Waveshapers::gloubi_boulga, false));
	_shapers.push_back (new Shaper (Waveshapers::gloubi_boulga_simple, false));
	_shapers.push_back (new Shaper (Waveshapers::warmth, false));
	_shapers.push_back (new Shaper (Waveshapers::saturation_1, true));

	QLabel* type_label = new QLabel ("Waveshaper type:", parent_widget());

	_waveshaper_type_combo = new QComboBox (parent_widget());
	_waveshaper_type_combo->addItem ("Gloubi-boulga", qVariantFromValue (_shapers[0]));
	_waveshaper_type_combo->addItem ("Gloubi-boulga simple (fast)", qVariantFromValue (_shapers[1]));
	_waveshaper_type_combo->addItem ("Warmth", qVariantFromValue (_shapers[2]));
	_waveshaper_type_combo->addItem ("Saturation", qVariantFromValue (_shapers[3]));
	QObject::connect (_waveshaper_type_combo, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_waveshaper_type_combo, SIGNAL (activated (int)), this, SLOT (update_widgets()));

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	_port_group = new Haruhi::PortGroup (_mikuru->graph(), QString ("Waveshaper %1").arg (this->id()).toStdString());
	_port_gain = new Haruhi::EventPort (_mikuru, "Gain", Haruhi::Port::Input, _port_group);
	_port_parameter = new Haruhi::EventPort (_mikuru, "Parameter", Haruhi::Port::Input, _port_group);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();

	_knob_gain = new Haruhi::Knob (parent_widget(), _port_gain, &_params.gain, "Gain", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Waveshaper::Gain, 100), 2);
	_knob_parameter = new Haruhi::Knob (parent_widget(), _port_parameter, &_params.parameter, "Parameter", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Waveshaper::Parameter, 100), 2);

	QGridLayout* type_layout = new QGridLayout();
	type_layout->setSpacing (Config::Spacing);
	type_layout->addWidget (type_label, 0, 0);
	type_layout->addWidget (_waveshaper_type_combo, 0, 1);
	type_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding), 1, 0);

	QHBoxLayout* layout = new QHBoxLayout (parent_widget());
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addLayout (type_layout);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	layout->addWidget (_knob_gain);
	layout->addWidget (_knob_parameter);

	update_params();
	update_widgets();
}


Waveshaper::~Waveshaper()
{
	_mikuru->free_id ("waveshapers", _id);

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
Waveshaper::process (Haruhi::AudioBuffer* buffer, unsigned int channel)
{
	Shaper* shaper = _waveshaper_type.load();
	if (!shaper)
		return;

	Shaper::Function f = shaper->function;
	float gain = _params.gain.to_f();
	float parameter = _params.parameter.to_f();

	for (Sample* s = buffer->begin(); s != buffer->end(); ++s)
		*s = (*f) (gain * *s, parameter);
}


void
Waveshaper::load_params()
{
	// Copy params:
	Params::Waveshaper p (_params);
	_loading_params = true;

	Effect::load_params();

	_waveshaper_type_combo->setCurrentIndex (p.type);

	_loading_params = false;
}


void
Waveshaper::load_params (Params::Waveshaper& params)
{
	_params = params;
	load_params();
}


void
Waveshaper::update_params()
{
	if (_loading_params)
		return;

	Effect::update_params();

	_params.type.set (_waveshaper_type_combo->currentIndex());
	_waveshaper_type.store (_waveshaper_type_combo->itemData (_params.type.get()).value<Shaper*> ());

	// Knob params are updated automatically using #assign_parameter.

	_params.sanitize();
}


void
Waveshaper::set_type (int type)
{
}


void
Waveshaper::update_widgets()
{
	_knob_parameter->setEnabled (_waveshaper_type.load()->has_parameter);
}

} // namespace MikuruPrivate

