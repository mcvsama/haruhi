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
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QToolTip>
#include <QtGui/QLabel>

// Local:
#include "mikuru.h"
#include "filter.h"
#include "common_filters.h"
#include "double_filter.h"


namespace MikuruPrivate {

CommonFilters::CommonFilters (Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_loading_params (false),
	_double_filter_1 (mikuru),
	_double_filter_2 (mikuru)
{
	Params::CommonFilters p = _params;

	_filter_ports = new Haruhi::PortGroup (_mikuru->graph(), "Common filters");

	_filter1 = new Filter (Filter::Filter1, _filter_ports, "Filter 1", "Filter 1 (2-pole IIR)", 0, _mikuru, this);
	_filter2 = new Filter (Filter::Filter2, _filter_ports, "Filter 2", "Filter 2 (2-pole IIR)", 0, _mikuru, this);

	_filter_configuration = new QComboBox (this);
	_filter_configuration->insertItem ("Serial", Params::CommonFilters::FilterConfigurationSerial);
	_filter_configuration->insertItem ("Parallel", Params::CommonFilters::FilterConfigurationParallel);
	_filter_configuration->setCurrentItem (p.filter_configuration);
	QObject::connect (_filter_configuration, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_filter_configuration, SIGNAL (activated (int)), this, SLOT (update_widgets()));

	_route_audio_input = new QCheckBox ("Route audio input", this);
	_route_audio_input->setChecked (p.route_audio_input);
	QObject::connect (_route_audio_input, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	// Layouts:

	QVBoxLayout* v1 = new QVBoxLayout (this, Config::Margin, Config::Spacing);
	QHBoxLayout* h1 = new QHBoxLayout (v1, Config::Spacing);
	h1->addWidget (new QLabel ("Configuration:", this));
	h1->addWidget (_filter_configuration);
	h1->addItem (new QSpacerItem (20, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
	h1->addWidget (_route_audio_input);
	h1->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	v1->addWidget (_filter1);
	v1->addWidget (_filter2);
	v1->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}


CommonFilters::~CommonFilters()
{
}


void
CommonFilters::reset()
{
	_double_filter_1.reset();
	_double_filter_2.reset();
}


void
CommonFilters::delete_ports()
{
	delete _filter1;
	delete _filter2;
	delete _filter_ports;
}


void
CommonFilters::process_events()
{
	_filter1->process_events();
	_filter2->process_events();
}


void
CommonFilters::process_filters (Haruhi::AudioBuffer& input1, Haruhi::AudioBuffer& buffer1, Haruhi::AudioBuffer& output1,
								Haruhi::AudioBuffer& input2, Haruhi::AudioBuffer& buffer2, Haruhi::AudioBuffer& output2)
{
	_double_filter_1.configure (static_cast<DoubleFilter::Configuration> (_params.filter_configuration.get()), _filter1->params(), _filter2->params());
	_double_filter_2.configure (static_cast<DoubleFilter::Configuration> (_params.filter_configuration.get()), _filter1->params(), _filter2->params());

	if (!_double_filter_1.process (input1, buffer1, buffer2, output1))
		output1.fill (&input1);
	if (!_double_filter_2.process (input2, buffer1, buffer2, output2))
		output2.fill (&input2);
}



void
CommonFilters::update_widgets()
{
	// TODO update scales according to number of passes	
}


void
CommonFilters::unit_bay_assigned()
{
	_filter1->unit_bay_assigned();
	_filter2->unit_bay_assigned();
}


void
CommonFilters::load_params()
{
	Params::CommonFilters p = _params;
	_loading_params = true;

	_filter_configuration->setCurrentItem (p.filter_configuration);
	_route_audio_input->setChecked (p.route_audio_input);

	_loading_params = false;
	update_widgets();
}


void
CommonFilters::load_params (Params::CommonFilters& params)
{
	_params = params;
	load_params();
}


void
CommonFilters::update_params()
{
	if (_loading_params)
		return;

	Params::CommonFilters p;
	p.filter_configuration.set (_filter_configuration->currentItem());
	p.route_audio_input.set (_route_audio_input->isChecked());
	_params = p;
}

} // namespace MikuruPrivate

