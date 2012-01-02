/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QToolTip>
#include <QtGui/QLabel>

// Local:
#include "mikuru.h"
#include "filter.h"
#include "part.h"
#include "part_filters.h"
#include "double_filter.h"


namespace MikuruPrivate {

/*
 * Methods for updating individual Voice parameters
 * instead of all parameters at once.
 */

#define UPDATE_VOICE(filter, param_name) \
	void PartFilters::update_##filter##_##param_name() \
	{ \
		if (_loading_params) \
			return; \
		_part->voice_manager()->set_##filter##_param (Haruhi::OmniVoice, &Params::Filter::param_name, _##filter->params()->param_name.get()); \
	}

UPDATE_VOICE (filter1, frequency)
UPDATE_VOICE (filter1, resonance)
UPDATE_VOICE (filter1, gain)
UPDATE_VOICE (filter1, attenuation)
UPDATE_VOICE (filter2, frequency)
UPDATE_VOICE (filter2, resonance)
UPDATE_VOICE (filter2, gain)
UPDATE_VOICE (filter2, attenuation)

#undef UPDATE_VOICE


PartFilters::PartFilters (Part* part, Haruhi::PortGroup* filter_ports, QString const& port_prefix, Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_part (part),
	_loading_params (false),
	_double_filter (mikuru)
{
	Params::PartFilters p = _params;

	_filter1 = new Filter (Filter::Filter1, filter_ports, port_prefix + " - 1", "Filter 1 (2-pole IIR)", _part, _mikuru, this);
	_filter2 = new Filter (Filter::Filter2, filter_ports, port_prefix + " - 2", "Filter 2 (2-pole IIR)", _part, _mikuru, this);

	// Filter configuration:

	_filter_configuration = new QComboBox (this);
	_filter_configuration->insertItem ("Serial", Params::PartFilters::FilterConfigurationSerial);
	_filter_configuration->insertItem ("Parallel", Params::PartFilters::FilterConfigurationParallel);
	_filter_configuration->setCurrentItem (p.filter_configuration);
	QObject::connect (_filter_configuration, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_filter_configuration, SIGNAL (activated (int)), this, SLOT (update_widgets()));

	// Layouts:

	QHBoxLayout* top_filters_layout = new QHBoxLayout();
	top_filters_layout->setSpacing (Config::Spacing);
	top_filters_layout->addWidget (new QLabel ("Configuration:", this));
	top_filters_layout->addWidget (_filter_configuration);
	top_filters_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	QVBoxLayout* filters_layout = new QVBoxLayout (this);
	filters_layout->setMargin (Config::Margin);
	filters_layout->setSpacing (Config::Spacing);
	filters_layout->addLayout (top_filters_layout);
	filters_layout->addWidget (_filter1);
	filters_layout->addWidget (_filter2);
	filters_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	_filter1->unit_bay_assigned();
	_filter2->unit_bay_assigned();

	setup_params();
}


PartFilters::~PartFilters()
{
	Signal::Receiver::disconnect_all_signals();
}


void
PartFilters::reset()
{
	_double_filter.reset();
}


void
PartFilters::delete_ports()
{
	delete _filter1;
	delete _filter2;
}


void
PartFilters::process_events()
{
	_filter1->process_events();
	_filter2->process_events();
}


void
PartFilters::load_params()
{
	Params::PartFilters p = _params;
	_loading_params = true;

	_filter_configuration->setCurrentItem (p.filter_configuration);

	_loading_params = false;
	update_widgets();
}


void
PartFilters::load_params (Params::PartFilters& params)
{
	_params = params;
	load_params();
}


void
PartFilters::update_params()
{
	if (_loading_params)
		return;

	Params::PartFilters p;
	p.filter_configuration = _filter_configuration->currentItem();
	_params = p;
}


void
PartFilters::update_widgets()
{
}


void
PartFilters::setup_params()
{
	// Controller params:
	_filter1->params()->frequency.on_change.connect (this, &PartFilters::update_filter1_frequency);
	_filter1->params()->resonance.on_change.connect (this, &PartFilters::update_filter1_resonance);
	_filter1->params()->gain.on_change.connect (this, &PartFilters::update_filter1_gain);
	_filter1->params()->attenuation.on_change.connect (this, &PartFilters::update_filter1_attenuation);

	// Controller params:
	_filter2->params()->frequency.on_change.connect (this, &PartFilters::update_filter2_frequency);
	_filter2->params()->resonance.on_change.connect (this, &PartFilters::update_filter2_resonance);
	_filter2->params()->gain.on_change.connect (this, &PartFilters::update_filter2_gain);
	_filter2->params()->attenuation.on_change.connect (this, &PartFilters::update_filter2_attenuation);
}

} // namespace MikuruPrivate

