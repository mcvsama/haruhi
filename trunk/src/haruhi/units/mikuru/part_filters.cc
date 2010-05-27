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
#include "part.h"
#include "part_filters.h"
#include "double_filter.h"


namespace MikuruPrivate {

PartFilters::PartFilters (Part* part, Core::PortGroup* filter_ports, QString const& port_prefix, Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_part (part),
	_loading_params (false),
	_double_filter (mikuru)
{
	Params::PartFilters p = _params;

	_filter1 = new Filter (Filter::Filter1, filter_ports, port_prefix + " - 1", "Filter 1 (2-pole IIR)", _part, _mikuru, this);
	_filter1->params_updated.connect (this, &PartFilters::filter1_params_updated);

	_filter2 = new Filter (Filter::Filter2, filter_ports, port_prefix + " - 2", "Filter 2 (2-pole IIR)", _part, _mikuru, this);
	_filter2->params_updated.connect (this, &PartFilters::filter2_params_updated);

	// Filter configuration:

	_filter_configuration = new QComboBox (this);
	_filter_configuration->insertItem ("Serial", Params::PartFilters::FilterConfigurationSerial);
	_filter_configuration->insertItem ("Parallel", Params::PartFilters::FilterConfigurationParallel);
	_filter_configuration->setCurrentItem (p.filter_configuration);
	QObject::connect (_filter_configuration, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_filter_configuration, SIGNAL (activated (int)), this, SLOT (update_widgets()));

	// Layouts:

	QVBoxLayout* filters_layout = new QVBoxLayout (this, Config::margin, Config::spacing);
	QHBoxLayout* top_filters_layout = new QHBoxLayout (filters_layout, Config::spacing);
	top_filters_layout->addWidget (new QLabel ("Configuration:", this));
	top_filters_layout->addWidget (_filter_configuration);
	top_filters_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	filters_layout->addWidget (_filter1);
	filters_layout->addWidget (_filter2);
	filters_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}


PartFilters::~PartFilters()
{
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
PartFilters::process_filters (Core::AudioBuffer& input, Core::AudioBuffer& buffer1, Core::AudioBuffer& buffer2, Core::AudioBuffer& output)
{
	_double_filter.configure (static_cast<DoubleFilter::Configuration> (static_cast<int> (atomic (_params.filter_configuration))), _filter1->params(), _filter2->params());

	if (!_double_filter.process (input, buffer1, buffer2, output))
		output.fill (&input);
}


void
PartFilters::unit_bay_assigned()
{
	_filter1->unit_bay_assigned();
	_filter2->unit_bay_assigned();
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
	// TODO update scales according to number of passes	
}


void
PartFilters::filter1_params_updated()
{
	_part->voice_manager()->set_all_filters1_params (*_filter1->params());
}


void
PartFilters::filter2_params_updated()
{
	_part->voice_manager()->set_all_filters2_params (*_filter2->params());
}

} // namespace MikuruPrivate

