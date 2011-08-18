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

#ifndef HARUHI__PLUGINS__MIKURU__GENERAL_H__INCLUDED
#define HARUHI__PLUGINS__MIKURU__GENERAL_H__INCLUDED


// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "widgets.h"
#include "envelopes.h"
#include "params.h"


class Mikuru;

namespace MikuruPrivate {

class General: public QWidget
{
	Q_OBJECT

	friend class Patch;

  public:
	General (Mikuru* mikuru, QWidget* parent);

	~General();

	void
	delete_ports();

	void
	delete_envelopes();

	Params::General*
	params() { return &_params; }

	void
	process_events();

	void
	unit_bay_assigned();

	Envelopes*
	envelopes() const { return _envelopes; }

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
	load_params (Params::General& params);

	/**
	 * Updates Params structure from widgets.
	 */
	void
	update_params();

  private:
	Mikuru*				_mikuru;
	Params::General		_params;
	bool				_loading_params;

	QSpinBox*			_polyphony;
	QCheckBox*			_enable_audio_input;

	// Ports:
	Haruhi::EventPort*	_port_volume;
	Haruhi::EventPort*	_port_detune;
	Haruhi::EventPort*	_port_panorama;
	Haruhi::EventPort*	_port_stereo_width;
	Haruhi::EventPort*	_port_input_volume;
	Haruhi::EventPort*	_port_pitchbend;
	Haruhi::EventPort*	_port_amplitude;
	Haruhi::EventPort*	_port_frequency;

	// Knobs:
	Haruhi::Knob*		_knob_volume;
	Haruhi::Knob*		_knob_detune;
	Haruhi::Knob*		_knob_panorama;
	Haruhi::Knob*		_knob_stereo_width;
	Haruhi::Knob*		_knob_input_volume;

	Envelopes*			_envelopes;
};

} // namespace MikuruPrivate

#endif

