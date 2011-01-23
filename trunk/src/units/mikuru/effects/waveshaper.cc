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

// Qt:
#include <QtGui/QLayout>

// Local:
#include "../mikuru.h"
#include "../params.h"
#include "waveshaper.h"


namespace MikuruPrivate {

Waveshaper::Waveshaper (int id, Mikuru* mikuru, QWidget* parent):
	Effect (parent),
	_mikuru (mikuru)
{
	_id = (id == 0) ? _mikuru->allocate_id ("waveshapers") : _mikuru->reserve_id ("waveshapers", id);

	_waveshaper_type_combo = new QComboBox (parent_widget());
	_waveshaper_type_combo->addItem ("Gloubi-boulga");
	_waveshaper_type_combo->addItem ("Gloubi-boulga simple");
	_waveshaper_type_combo->addItem ("Warmth");
	QObject::connect (_waveshaper_type_combo, SIGNAL (activated (int)), this, SLOT (set_type (int)));

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	_port_group = new Haruhi::PortGroup (_mikuru->graph(), QString ("Waveshaper %1").arg (this->id()).toStdString());
	_port_gain = new Haruhi::EventPort (_mikuru, "Gain", Haruhi::Port::Input, _port_group);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();

	_knob_gain = new Haruhi::Knob (parent_widget(), _port_gain, &_params.gain, "Gain", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Waveshaper::Gain, 100), 2);

	QHBoxLayout* layout = new QHBoxLayout (parent_widget());
	layout->setMargin (Config::Margin);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (_waveshaper_type_combo);
	layout->addWidget (_knob_gain);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
}


Waveshaper::~Waveshaper()
{
	_mikuru->free_id ("waveshapers", _id);

	delete _knob_gain;

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	delete _port_gain;
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
Waveshaper::set_type (int type)
{
	_waveshaper_type.store (type);
}


void
Waveshaper::process (Haruhi::AudioBuffer* buffer, unsigned int channel)
{
	Sample (Waveshaper::*x)(Sample) = 0;
	switch (_waveshaper_type.load())
	{
		case 0:	x = &Waveshaper::waveshape_gloubi_boulga; break;
		case 1:	x = &Waveshaper::waveshape_gloubi_boulga_simple; break;
		case 2:	x = &Waveshaper::waveshape_warmth; break;
	}
	float gain = _params.gain.to_f();
	for (Sample* s = buffer->begin(); s != buffer->end(); ++s)
		*s = (this->*x) (gain * *s);
}

} // namespace MikuruPrivate

