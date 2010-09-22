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

// Qt:
#include <QtGui/QToolTip>
#include <Qt3Support/Q3GroupBox>

// Haruhi:
#include <haruhi/controller_proxy.h>
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

	_mikuru->graph()->lock();
	_port_volume = new Core::EventPort (_mikuru, "Volume", Core::Port::Input, 0, Core::Port::ControlVolume);
	_port_detune = new Core::EventPort (_mikuru, "Detune", Core::Port::Input);
	_port_panorama = new Core::EventPort (_mikuru, "Panorama", Core::Port::Input);
	_port_stereo_width = new Core::EventPort (_mikuru, "Stereo width", Core::Port::Input);
	_port_input_volume = new Core::EventPort (_mikuru, "Input volume", Core::Port::Input);
	_port_pitchbend = new Core::EventPort (_mikuru, "Pitchbend (global)", Core::Port::Input, 0, Core::Port::ControlPitchbend | Core::Port::Polyphonic);
	_port_amplitude= new Core::EventPort (_mikuru, "Amplitude (global)", Core::Port::Input, 0, Core::Port::ControlPitchbend | Core::Port::Polyphonic);
	_port_frequency = new Core::EventPort (_mikuru, "Frequency (global)", Core::Port::Input, 0, Core::Port::ControlPitchbend | Core::Port::Polyphonic);
	_mikuru->graph()->unlock();

	_proxy_volume = new Haruhi::ControllerProxy (_port_volume, &_params.volume);
	_proxy_detune = new Haruhi::ControllerProxy (_port_detune, &_params.detune);
	_proxy_panorama = new Haruhi::ControllerProxy (_port_panorama, &_params.panorama);
	_proxy_stereo_width = new Haruhi::ControllerProxy (_port_stereo_width, &_params.stereo_width);
	_proxy_input_volume = new Haruhi::ControllerProxy (_port_input_volume, &_params.input_volume);

	_control_volume = new Haruhi::Knob (this, _proxy_volume, "Volume", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::General::Volume, 100), 2);
	_control_detune = new Haruhi::Knob (this, _proxy_detune, "Detune", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::General::Detune, 100), 2);
	_control_panorama = new Haruhi::Knob (this, _proxy_panorama, "Panorama", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::General::Panorama, 100), 2);
	_control_stereo_width = new Haruhi::Knob (this, _proxy_stereo_width, "St.width", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::General::StereoWidth, 100), 2);
	_control_input_volume = new Haruhi::Knob (this, _proxy_input_volume, "Input vol.", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::General::InputVolume, 100), 2);
	QToolTip::add (_control_stereo_width, "Stereo width");

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
	_polyphony->setValue (atomic (_params.polyphony));
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
	h1_layout->addWidget (_control_volume);
	h1_layout->addWidget (_control_detune);
	h1_layout->addWidget (_control_panorama);
	h1_layout->addWidget (_control_stereo_width);
	h1_layout->addWidget (_control_input_volume);
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
	delete _control_volume;
	delete _control_detune;
	delete _control_panorama;
	delete _control_stereo_width;
	delete _control_input_volume;

	delete _proxy_volume;
	delete _proxy_detune;
	delete _proxy_panorama;
	delete _proxy_stereo_width;
	delete _proxy_input_volume;

	_mikuru->graph()->lock();
	delete _port_volume;
	delete _port_detune;
	delete _port_panorama;
	delete _port_stereo_width;
	delete _port_input_volume;
	delete _port_pitchbend;
	delete _port_amplitude;
	delete _port_frequency;
	_mikuru->graph()->unlock();
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
	_proxy_volume->process_events();
	_proxy_detune->process_events();
	_proxy_panorama->process_events();
	_proxy_stereo_width->process_events();
	_proxy_input_volume->process_events();

	// Copy values from pitchbend/amplitude/frequency ports to each part's port:
	Core::EventBuffer* pitchbend_buffer = _port_pitchbend->event_buffer();
	Core::EventBuffer* amplitude_buffer = _port_amplitude->event_buffer();
	Core::EventBuffer* frequency_buffer = _port_frequency->event_buffer();
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
	_control_volume->set_unit_bay (_mikuru->unit_bay());
	_control_detune->set_unit_bay (_mikuru->unit_bay());
	_control_panorama->set_unit_bay (_mikuru->unit_bay());
	_control_stereo_width->set_unit_bay (_mikuru->unit_bay());
	_control_input_volume->set_unit_bay (_mikuru->unit_bay());
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

	atomic (_params.polyphony) = _polyphony->value();
	atomic (_params.enable_audio_input) = _enable_audio_input->isChecked();

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

