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

#ifndef HARUHI__UNITS__MIKURU__PART_FILTERS_H__INCLUDED
#define HARUHI__UNITS__MIKURU__PART_FILTERS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/core/port_group.h>
#include <haruhi/utility/signal.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "filter.h"
#include "widgets.h"
#include "double_filter.h"
#include "params.h"


class Mikuru;

namespace MikuruPrivate {

class Filter;

class PartFilters:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

  public:
	PartFilters (Part* part, Haruhi::PortGroup* filter_ports, QString const& port_prefix, Mikuru* mikuru, QWidget* parent);

	~PartFilters();

	void
	reset();

	void
	delete_ports();

	Filter*
	filter1() const { return _filter1; }

	Filter*
	filter2() const { return _filter2; }

	Params::PartFilters*
	params() { return &_params; }

	void
	process_events();

	void
	process_filters (Haruhi::AudioBuffer& input, Haruhi::AudioBuffer& buffer1, Haruhi::AudioBuffer& buffer2, Haruhi::AudioBuffer& output);

	void
	unit_bay_assigned();

	Mikuru*
	mikuru() const { return _mikuru; }

  public slots:
	/**
	 * Loads widgets values from Params struct.
	 */
	void
	load_params();

	/**
	 * Loads params from given struct and updates widgets.
	 */
	void
	load_params (Params::PartFilters& params);

	/**
	 * Updates Params structure from widgets.
	 */
	void
	update_params();

	/**
	 * Updates widgets.
	 */
	void
	update_widgets();

	void
	filter1_params_updated();

	void
	filter2_params_updated();

  private:
	Mikuru*					_mikuru;
	Part*					_part;
	Params::PartFilters		_params;
	bool					_loading_params;

	Haruhi::PortGroup*		_filter_ports;
	DoubleFilter			_double_filter;

	Filter*					_filter1;
	Filter*					_filter2;
	QComboBox*				_filter_configuration;
};

} // namespace MikuruPrivate

#endif

