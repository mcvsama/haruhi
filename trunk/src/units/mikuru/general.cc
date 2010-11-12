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
#include <numeric>

// Qt:
#include <QtGui/QToolTip>
#include <Qt3Support/Q3GroupBox>

// Haruhi:
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "mikuru.h"
#include "general.h"
#include "oscillator.h"
#include "widgets.h"
#include "params.h"


namespace MikuruPrivate {

General::General (Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_loading_params (false),
	_dont_notify_threads_number (false)
{
	Params::General p = _params;

	_port_volume = new Haruhi::EventPort (_mikuru, "Volume", Haruhi::Port::Input, 0, Haruhi::Port::ControlVolume);
	_port_detune = new Haruhi::EventPort (_mikuru, "Detune", Haruhi::Port::Input);
	_port_panorama = new Haruhi::EventPort (_mikuru, "Panorama", Haruhi::Port::Input);
	_port_stereo_width = new Haruhi::EventPort (_mikuru, "Stereo width", Haruhi::Port::Input);
	_port_input_volume = new Haruhi::EventPort (_mikuru, "Input volume", Haruhi::Port::Input);
	_port_pitchbend = new Haruhi::EventPort (_mikuru, "Pitchbend (global)", Haruhi::Port::Input, 0, Haruhi::Port::ControlPitchbend | Haruhi::Port::Polyphonic);
	_port_amplitude= new Haruhi::EventPort (_mikuru, "Amplitude (global)", Haruhi::Port::Input, 0, Haruhi::Port::ControlPitchbend | Haruhi::Port::Polyphonic);
	_port_frequency = new Haruhi::EventPort (_mikuru, "Frequency (global)", Haruhi::Port::Input, 0, Haruhi::Port::ControlPitchbend | Haruhi::Port::Polyphonic);

	_knob_volume = new Haruhi::Knob (this, _port_volume, &_params.volume, "Volume dB",
									 -std::numeric_limits<float>::infinity(), 0.0f,
									 (Params::General::VolumeMax - Params::General::VolumeMin) / 500, 2);
	_knob_detune = new Haruhi::Knob (this, _port_detune, &_params.detune, "Detune", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::General::Detune, 100), 2);
	_knob_panorama = new Haruhi::Knob (this, _port_panorama, &_params.panorama, "Panorama", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::General::Panorama, 100), 2);
	_knob_stereo_width = new Haruhi::Knob (this, _port_stereo_width, &_params.stereo_width, "St.width", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::General::StereoWidth, 100), 2);
	_knob_input_volume = new Haruhi::Knob (this, _port_input_volume, &_params.input_volume, "Input dB",
										   -std::numeric_limits<float>::infinity(), 0.0f,
										   (Params::General::InputVolumeMax - Params::General::InputVolumeMin) / 500, 2);

	_knob_volume->set_volume_scale (true, M_E);
	_knob_input_volume->set_volume_scale (true, M_E);

	QToolTip::add (_knob_stereo_width, "Stereo width");

	Q3GroupBox* grid1 = new Q3GroupBox (2, Qt::Horizontal, "", this);
	grid1->setInsideMargin (3 * Config::margin);

	// Threads:

	new QLabel ("CPU threads:", grid1);
	_threads_number = new QSpinBox (0, 16, 1, grid1);
	_threads_number->setSpecialValueText ("Auto");
	_threads_number->setValue (0);
	QObject::connect (_threads_number, SIGNAL (valueChanged (int)), this, SLOT (update_threads (int)));

	// Polyphony:

	new QLabel ("Polyphony (each part):", grid1);
	_polyphony = new QSpinBox (1, 256, 1, grid1);
	_polyphony->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	_polyphony->setValue (_params.polyphony.get());
	QObject::connect (_polyphony, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	// Enable audio input:

	_enable_audio_input = new QCheckBox ("Enable audio input", grid1);
	_enable_audio_input->setChecked (_params.enable_audio_input);
	QObject::connect (_enable_audio_input, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	grid1->addSpace (0);

	// Envelopes:

	_envelopes = new Envelopes (_mikuru, this);

	// Layouts:

	QVBoxLayout* v1_layout = new QVBoxLayout (this, Config::margin, Config::spacing);
	QHBoxLayout* h1_layout = new QHBoxLayout (v1_layout, Config::spacing);
	h1_layout->addWidget (_knob_volume);
	h1_layout->addWidget (_knob_detune);
	h1_layout->addWidget (_knob_panorama);
	h1_layout->addWidget (_knob_stereo_width);
	h1_layout->addWidget (_knob_input_volume);
	h1_layout->addWidget (grid1);
	v1_layout->addWidget (new StyledLabel ("LFOs & Envelopes", this));
	v1_layout->addWidget (_envelopes);
}


General::~General()
{
	// Ports and proxies deleted in delete_ports() by Mikuru.
}


void
General::delete_ports()
{
	// Delete knobs before ControllerProxies:
	delete _knob_volume;
	delete _knob_detune;
	delete _knob_panorama;
	delete _knob_stereo_width;
	delete _knob_input_volume;

	delete _port_volume;
	delete _port_detune;
	delete _port_panorama;
	delete _port_stereo_width;
	delete _port_input_volume;
	delete _port_pitchbend;
	delete _port_amplitude;
	delete _port_frequency;
}


void
General::delete_envelopes()
{
	_envelopes->hide();
	delete _envelopes;
}


void
General::process_events()
{
	_knob_volume->controller_proxy().process_events();
	_knob_detune->controller_proxy().process_events();
	_knob_panorama->controller_proxy().process_events();
	_knob_stereo_width->controller_proxy().process_events();
	_knob_input_volume->controller_proxy().process_events();

	// Copy values from pitchbend/amplitude/frequency ports to each part's port:
	Haruhi::EventBuffer* pitchbend_buffer = _port_pitchbend->event_buffer();
	Haruhi::EventBuffer* amplitude_buffer = _port_amplitude->event_buffer();
	Haruhi::EventBuffer* frequency_buffer = _port_frequency->event_buffer();
	for (Mikuru::Parts::iterator p = _mikuru->parts().begin(); p != _mikuru->parts().end(); ++p)
	{
		(*p)->oscillator()->pitchbend_port()->event_buffer()->mixin (pitchbend_buffer);
		(*p)->oscillator()->amplitude_port()->event_buffer()->mixin (amplitude_buffer);
		(*p)->oscillator()->frequency_port()->event_buffer()->mixin (frequency_buffer);
	}
	pitchbend_buffer->clear();
	amplitude_buffer->clear();
	frequency_buffer->clear();
}


void
General::unit_bay_assigned()
{
	_knob_volume->set_unit_bay (_mikuru->unit_bay());
	_knob_detune->set_unit_bay (_mikuru->unit_bay());
	_knob_panorama->set_unit_bay (_mikuru->unit_bay());
	_knob_stereo_width->set_unit_bay (_mikuru->unit_bay());
	_knob_input_volume->set_unit_bay (_mikuru->unit_bay());
}


void
General::set_threads_number (int threads)
{
	_dont_notify_threads_number = true;
	_threads_number->setValue (threads);
	update_threads (threads);
	_dont_notify_threads_number = false;
}


void
General::load_params()
{
	// Copy params:
	Params::General p (_params);
	_loading_params = true;

	_polyphony->setValue (p.polyphony);
	_enable_audio_input->setChecked (p.enable_audio_input);

	_loading_params = false;
}


void
General::load_params (Params::General& params)
{
	_params = params;
	load_params();
}


void
General::update_params()
{
	if (_loading_params)
		return;

	_params.polyphony.set (_polyphony->value());
	_params.enable_audio_input.set (_enable_audio_input->isChecked());

	// Knob params are updated automatically using #assign_parameter.
}


void
General::update_threads (int threads)
{
	_mikuru->set_threads_number (threads);
	// Save only when fully constructed:
	if (_mikuru->enabled())
	{
		_mikuru->save_config();
		// Notify other Mikurus about changed configuration:
		if (!_dont_notify_threads_number)
			_mikuru->graph()->notify (new Mikuru::UpdateConfig (_mikuru, _mikuru->urn()));
	}
}

} // namespace MikuruPrivate

