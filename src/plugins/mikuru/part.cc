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
#include <set>

// Qt:
#include <QtGui/QWhatsThis>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "mikuru.h"
#include "part.h"
#include "oscillator.h"
#include "filter.h"
#include "part_filters.h"
#include "part_effects.h"


namespace MikuruPrivate {

Part::Part (Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_loading_params (false)
{
	// Find lowest ID and use it:
	_id = _mikuru->allocate_id ("parts");

	Params::Part p = _params;
	_voice_manager = new VoiceManager (this);
	_port_group = new Haruhi::PortGroup (_mikuru->graph(), QString ("Part %1").arg (id()).toStdString());

	_buffer_1 = new Haruhi::AudioBuffer();
	_buffer_2 = new Haruhi::AudioBuffer();

	// Tabs/widgets:

	QTabWidget* tabs = new QTabWidget (this);

	_oscillator = new Oscillator (this, _port_group, "Oscillator", _mikuru, tabs);
	_filters = new PartFilters (this, _port_group, "Filter", _mikuru, tabs);
	_effects = new PartEffects (this, _mikuru, tabs);

	// Top part widgets:

	_part_enabled = new StyledCheckBoxLabel ("Enabled", this);
	_part_enabled->checkbox()->setChecked (p.enabled);
	QObject::connect (_part_enabled->checkbox(), SIGNAL (clicked()), this, SLOT (update_params()));

	// Add tabs:
	tabs->addTab (_oscillator, Resources::Icons16::wave_sine(), "Oscillator");
	tabs->addTab (new QWidget(), "Modulator");
	tabs->addTab (_filters, Resources::Icons16::filters(), "Filters");
	tabs->addTab (_effects, Resources::Icons16::effects(), "Effects");
	tabs->showPage (_oscillator);

	QVBoxLayout* layout = new QVBoxLayout (this, Config::Margin, Config::Spacing);
	layout->addWidget (_part_enabled);
	layout->addWidget (tabs);
}


Part::~Part()
{
	_mikuru->free_id ("parts", _id);
	delete _voice_manager;
	delete _port_group;
	_filters->delete_ports();

	delete _buffer_1;
	delete _buffer_2;
}


void
Part::process_events()
{
	_oscillator->process_events();
	_filters->process_events();
}


void
Part::mix_voices()
{
	_buffer_1->resize (_mikuru->graph()->buffer_size());
	_buffer_2->resize (_mikuru->graph()->buffer_size());

	_buffer_1->clear();
	_buffer_2->clear();

	// Mix all voices into part's buffers:
	_voice_manager->mix_voices (_buffer_1, _buffer_2);
}


void
Part::process_effects()
{
	// Apply pseudo stereo effect:
	if (_oscillator->oscillator_params()->pseudo_stereo.get())
		_buffer_2->negate();

	_effects->process (_buffer_1, 0);
	_effects->process (_buffer_2, 1);
}


void
Part::graph_updated()
{
	_voice_manager->graph_updated();
}


void
Part::load_params()
{
	Params::Part p = _params;
	_loading_params = true;

	_part_enabled->checkbox()->setChecked (p.enabled);

	_loading_params = false;
}


void
Part::load_params (Params::Part& params)
{
	_params = params;
	load_params();
}


void
Part::update_params()
{
	if (_loading_params)
		return;

	Params::Part p;
	p.enabled = _part_enabled->checkbox()->isChecked();
	_params = p;
}

} // namespace MikuruPrivate

