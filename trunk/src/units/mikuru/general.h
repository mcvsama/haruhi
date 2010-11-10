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


#ifndef HARUHI__UNITS__MIKURU__GENERAL_H__INCLUDED
#define HARUHI__UNITS__MIKURU__GENERAL_H__INCLUDED


// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "widgets.h"
#include "envelopes.h"
#include "params.h"


class Mikuru;

namespace MikuruPrivate {

namespace Core = Haruhi::Core;


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

	int
	threads_number() const { return _threads_number->value(); }

	void
	set_threads_number (int threads);

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

	/**
	 * Called when user changes threads number.
	 */
	void
	update_threads (int threads);

  private:
	Mikuru*						_mikuru;
	Params::General				_params;
	bool						_loading_params;

	bool						_dont_notify_threads_number;
	QSpinBox*					_threads_number;
	QSpinBox*					_polyphony;
	QCheckBox*					_enable_audio_input;

	// Ports:
	Core::EventPort*			_port_volume;
	Core::EventPort*			_port_detune;
	Core::EventPort*			_port_panorama;
	Core::EventPort*			_port_stereo_width;
	Core::EventPort*			_port_input_volume;
	Core::EventPort*			_port_pitchbend;
	Core::EventPort*			_port_amplitude;
	Core::EventPort*			_port_frequency;

	// Knobs:
	Haruhi::Knob*				_knob_volume;
	Haruhi::Knob*				_knob_detune;
	Haruhi::Knob*				_knob_panorama;
	Haruhi::Knob*				_knob_stereo_width;
	Haruhi::Knob*				_knob_input_volume;

	Envelopes*					_envelopes;
};

} // namespace MikuruPrivate

#endif

