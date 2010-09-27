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

#ifndef HARUHI__UNITS__VANHALEN__VANHALEN_H__INCLUDED
#define HARUHI__UNITS__VANHALEN__VANHALEN_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>

// Local:
#include <haruhi/core/event.h>
#include <haruhi/core/event_port.h>
#include <haruhi/core/audio_port.h>
#include <haruhi/dsp/delay_line.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/unit.h>


class VanHalen:
	public Haruhi::Unit
{
	Q_OBJECT

  public:
	VanHalen (Haruhi::UnitFactory*, Haruhi::Session*, std::string const& urn, std::string const& title, int id, QWidget* parent);

	virtual ~VanHalen();

	std::string
	name() const;

	void
	process();

	void
	panic();

  private:
	// Drive:
	Core::EventPort*			_input;
	Core::EventPort*			_output;
	Core::AudioPort*			_audio_input_1;
	Core::AudioPort*			_audio_input_2;
	Core::AudioPort*			_audio_output_1;
	Core::AudioPort*			_audio_output_2;

	Core::AudioBuffer			_buf1;
	Core::AudioBuffer			_buf2;

	DSP::DelayLine				_delay1;
	DSP::DelayLine				_delay2;

	Haruhi::ControllerProxy*	_proxy_comb_index;
	Haruhi::ControllerProxy*	_proxy_comb_alpha;

	Haruhi::Knob*				_knob_comb_index;
	Haruhi::Knob*				_knob_comb_alpha;

	Haruhi::ControllerParam		_comb_index;
	Haruhi::ControllerParam		_comb_alpha;
};


class VanHalenFactory: public Haruhi::UnitFactory
{
  public:
	VanHalenFactory();

	Haruhi::Unit*
	create_unit (Haruhi::Session*, int id, QWidget* parent);

	void
	destroy_unit (Haruhi::Unit* unit);

	InformationMap const&
	information() const;

  private:
	InformationMap _information;
};

#endif

