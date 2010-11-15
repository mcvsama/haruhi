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
class Waveshaper;

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

	Waveform*
	waveform() const { return _waveform; }

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

	// Widgets:
	StyledCheckBoxLabel*	_part_enabled;
	Waveform*				_waveform;
	Oscillator*				_oscillator;
	PartFilters*			_filters;
};

} // namespace MikuruPrivate

#endif
