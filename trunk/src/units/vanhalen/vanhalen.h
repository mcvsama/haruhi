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
#include <haruhi/plugin/plugin.h>
#include <haruhi/plugin/plugin_factory.h>
#include <haruhi/widgets/knob.h>


class VanHalen:
	public Haruhi::Plugin
{
	Q_OBJECT

  public:
	VanHalen (Haruhi::Session*, std::string const& urn, std::string const& title, int id, QWidget* parent);

	virtual ~VanHalen();

	/**
	 * Core::Unit API
	 */
	void
	registered();

	/**
	 * Core::Unit API
	 */
	void
	unregistered();

	std::string
	name() const;

	void
	process();

	void
	panic();

  private:
	// Drive:
	Haruhi::Core::EventPort*	_input;
	Haruhi::Core::EventPort*	_output;
	Haruhi::Core::AudioPort*	_audio_input_1;
	Haruhi::Core::AudioPort*	_audio_input_2;
	Haruhi::Core::AudioPort*	_audio_output_1;
	Haruhi::Core::AudioPort*	_audio_output_2;

	Haruhi::Core::AudioBuffer	_buf1;
	Haruhi::Core::AudioBuffer	_buf2;

	Haruhi::DSP::DelayLine		_delay1;
	Haruhi::DSP::DelayLine		_delay2;

	Haruhi::Knob*				_knob_comb_index;
	Haruhi::Knob*				_knob_comb_alpha;

	Haruhi::ControllerParam		_comb_index;
	Haruhi::ControllerParam		_comb_alpha;
};


class VanHalenFactory: public Haruhi::PluginFactory
{
  public:
	VanHalenFactory();

	Haruhi::Plugin*
	create_plugin (Haruhi::Session*, int id, QWidget* parent);

	void
	destroy_plugin (Haruhi::Plugin* plugin);

	InformationMap const&
	information() const;

  private:
	InformationMap _information;
};

#endif

