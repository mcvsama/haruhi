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

#ifndef HARUHI__UNITS__MIKURU__COMMON_FILTERS_H__INCLUDED
#define HARUHI__UNITS__MIKURU__COMMON_FILTERS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/core/port_group.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "filter.h"
#include "widgets.h"
#include "double_filter.h"
#include "params.h"


class Mikuru;


namespace MikuruPrivate {

class CommonFilters: public QWidget
{
	Q_OBJECT

  public:
	CommonFilters (Mikuru* mikuru, QWidget* parent);

	~CommonFilters();

	void
	reset();

	void
	delete_ports();

	Filter*
	filter1() const { return _filter1; }

	Filter*
	filter2() const { return _filter2; }

	Params::CommonFilters*
	params() { return &_params; }

	void
	process_events();

	void
	process_filters (Haruhi::AudioBuffer& input1, Haruhi::AudioBuffer& buffer1, Haruhi::AudioBuffer& output1,
					 Haruhi::AudioBuffer& input2, Haruhi::AudioBuffer& buffer2, Haruhi::AudioBuffer& output2);

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
	load_params (Params::CommonFilters& params);

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

  private:
	Mikuru*					_mikuru;
	Params::CommonFilters	_params;
	bool					_loading_params;

	Haruhi::PortGroup*		_filter_ports;
	DoubleFilter			_double_filter_1;
	DoubleFilter			_double_filter_2;

	Filter*					_filter1;
	Filter*					_filter2;
	QComboBox*				_filter_configuration;
	QCheckBox*				_route_audio_input;
};

} // namespace MikuruPrivate

#endif

