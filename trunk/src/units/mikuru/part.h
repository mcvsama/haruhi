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

#ifndef HARUHI__UNITS__MIKURU__PART_H__INCLUDED
#define HARUHI__UNITS__MIKURU__PART_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/application/haruhi.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/wave.h>

// Local:
#include "voice_manager.h"
#include "widgets.h"
#include "params.h"


class Mikuru;

namespace MikuruPrivate {

class Waveform;
class Oscillator;
class PartFilters;
class PartEffects;

class Part: public QWidget
{
	Q_OBJECT

  public:
	Part (Mikuru* mikuru, QWidget* parent);

	~Part();

	int
	id() const { return _id; }

	void
	process_events();

	Params::Part*
	params() { return &_params; }

	VoiceManager*
	voice_manager() const { return _voice_manager; }

	Haruhi::PortGroup*
	port_group() const { return _port_group; }

	Mikuru*
	mikuru() const { return _mikuru; }

	Oscillator*
	oscillator() const { return _oscillator; }

	PartFilters*
	filters() const { return _filters; }

	PartEffects*
	effects() const { return _effects; }

	Haruhi::AudioBuffer*
	buffer1() const { return _buffer_1; }

	Haruhi::AudioBuffer*
	buffer2() const { return _buffer_2; }

	void
	prepare_buffers();

	void
	process_effects();

  public slots:
	void
	load_params();

	void
	load_params (Params::Part& params);

	void
	update_params();

  private:
	Mikuru*					_mikuru;
	Params::Part			_params;
	bool					_loading_params;
	int						_id;

	VoiceManager*			_voice_manager;
	Haruhi::PortGroup*		_port_group;

	// Output buffers:
	Haruhi::AudioBuffer*	_buffer_1;
	Haruhi::AudioBuffer*	_buffer_2;

	// Widgets:
	StyledCheckBoxLabel*	_part_enabled;
	Oscillator*				_oscillator;
	PartFilters*			_filters;
	PartEffects*			_effects;
};

} // namespace MikuruPrivate

#endif

