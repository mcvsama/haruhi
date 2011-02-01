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

#ifndef HARUHI__UNITS__VANHALEN__VANHALEN_H__INCLUDED
#define HARUHI__UNITS__VANHALEN__VANHALEN_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>

// Local:
#include <haruhi/graph/event.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/graph/audio_port.h>
#include <haruhi/graph/audio_buffer.h>
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
	VanHalen (std::string const& urn, std::string const& title, int id, QWidget* parent);

	virtual ~VanHalen();

	/**
	 * Unit API
	 */
	void
	registered();

	/**
	 * Unit API
	 */
	void
	unregistered();

	std::string
	name() const;

	void
	process();

	void
	panic();

	void
	graph_updated();

  private:
	// Drive:
	Haruhi::EventPort*		_input;
	Haruhi::EventPort*		_output;
	Haruhi::AudioPort*		_audio_input_1;
	Haruhi::AudioPort*		_audio_input_2;
	Haruhi::AudioPort*		_audio_output_1;
	Haruhi::AudioPort*		_audio_output_2;

	Haruhi::AudioBuffer		_buf1;
	Haruhi::AudioBuffer		_buf2;

	Haruhi::DSP::DelayLine	_delay1;
	Haruhi::DSP::DelayLine	_delay2;

	Haruhi::Knob*			_knob_comb_index;
	Haruhi::Knob*			_knob_comb_alpha;

	Haruhi::ControllerParam	_comb_index;
	Haruhi::ControllerParam	_comb_alpha;
};


class VanHalenFactory: public Haruhi::PluginFactory
{
  public:
	VanHalenFactory();

	Haruhi::Plugin*
	create_plugin (int id, QWidget* parent);

	void
	destroy_plugin (Haruhi::Plugin* plugin);

	InformationMap const&
	information() const;

  private:
	InformationMap _information;
};

#endif
