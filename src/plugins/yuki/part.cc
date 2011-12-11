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

// Standard:
#include <cstddef>
#include <utility>

// Lib:
#include <boost/bind.hpp>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/graph/event.h>
#include <haruhi/dsp/fft_filler.h>
#include <haruhi/dsp/functions.h>
#include <haruhi/dsp/modulated_wave.h>
#include <haruhi/utility/fast_pow.h>

// Local:
#include "part.h"
#include "part_manager.h"
#include "plugin.h"
#include "voice_manager.h"


namespace Yuki {

Part::UpdateWavetableWorkUnit::UpdateWavetableWorkUnit (Part* part):
	_part (part),
	_wavetable (0),
	_serial (0),
	_is_cancelled (false)
{ }


void
Part::UpdateWavetableWorkUnit::reset (DSP::Wavetable* wavetable, unsigned int serial)
{
	_wavetable = wavetable;
	_serial = serial;
	_is_cancelled.store (false);
}


void
Part::UpdateWavetableWorkUnit::execute()
{
	DSP::Wave* wave = _part->final_wave();

	DSP::FFTFiller filler (wave, true, 0.000001f);
	filler.set_cancel_predicate (boost::bind (&UpdateWavetableWorkUnit::is_cancelled, this));
	filler.fill (_wavetable, 4096);

	if (!filler.was_interrupted())
	{
		// We're sure that Part still exists as long as this object exist,
		// because Part will wait for us in its destructor.
		_part->wavetable_computed (_serial);
	}

	delete wave;
}


Part::PartPorts::PartPorts (Plugin* plugin, unsigned int part_id):
	HasPlugin (plugin)
{
	_port_group = new Haruhi::PortGroup (graph(), QString ("Part %1").arg (part_id).toStdString());

	if (graph())
		graph()->lock();
	wave_shape			= new Haruhi::EventPort (plugin, "Wave shape", Haruhi::Port::Input, _port_group);
	modulator_amplitude	= new Haruhi::EventPort (plugin, "Wave modulator amplitude", Haruhi::Port::Input, _port_group);
	modulator_index		= new Haruhi::EventPort (plugin, "Wave modulator index", Haruhi::Port::Input, _port_group);
	modulator_shape		= new Haruhi::EventPort (plugin, "Wave modulator shape", Haruhi::Port::Input, _port_group);
	volume				= new Haruhi::EventPort (plugin, "Volume", Haruhi::Port::Input, _port_group);
	amplitude			= new Haruhi::EventPort (plugin, "Amplitude modulation", Haruhi::Port::Input, _port_group, Haruhi::Port::Polyphonic);
	frequency			= new Haruhi::EventPort (plugin, "Frequency modulation", Haruhi::Port::Input, _port_group, Haruhi::Port::Polyphonic);
	panorama			= new Haruhi::EventPort (plugin, "Panorama", Haruhi::Port::Input, _port_group, Haruhi::Port::Polyphonic);
	detune				= new Haruhi::EventPort (plugin, "Detune", Haruhi::Port::Input, _port_group, Haruhi::Port::Polyphonic);
	pitchbend			= new Haruhi::EventPort (plugin, "Pitchbend", Haruhi::Port::Input, _port_group, Haruhi::Port::Polyphonic);
	velocity_sens		= new Haruhi::EventPort (plugin, "Velocity sensitivity", Haruhi::Port::Input, _port_group, Haruhi::Port::Polyphonic);
	unison_index		= new Haruhi::EventPort (plugin, "Unison index", Haruhi::Port::Input, _port_group, Haruhi::Port::Polyphonic);
	unison_spread		= new Haruhi::EventPort (plugin, "Unison spread", Haruhi::Port::Input, _port_group, Haruhi::Port::Polyphonic);
	unison_init			= new Haruhi::EventPort (plugin, "Unison init. φ", Haruhi::Port::Input, _port_group, Haruhi::Port::Polyphonic);
	unison_noise		= new Haruhi::EventPort (plugin, "Unison noise", Haruhi::Port::Input, _port_group, Haruhi::Port::Polyphonic);
	portamento_time		= new Haruhi::EventPort (plugin, "Portamento time", Haruhi::Port::Input, _port_group);
	phase				= new Haruhi::EventPort (plugin, "Phase", Haruhi::Port::Input, _port_group);
	noise_level			= new Haruhi::EventPort (plugin, "Noise level", Haruhi::Port::Input, _port_group);
	if (graph())
		graph()->unlock();
}


Part::PartPorts::~PartPorts()
{
// TODO check if 'if's are needed:
	if (graph())
		graph()->lock();
	delete wave_shape;
	delete modulator_amplitude;
	delete modulator_index;
	delete modulator_shape;
	delete volume;
	delete amplitude;
	delete frequency;
	delete panorama;
	delete detune;
	delete pitchbend;
	delete velocity_sens;
	delete unison_index;
	delete unison_spread;
	delete unison_init;
	delete unison_noise;
	delete portamento_time;
	delete phase;
	delete noise_level;
	if (graph())
		graph()->unlock();

	delete _port_group;
}


Part::Part (PartManager* part_manager, WorkPerformer* work_performer, Params::Main* main_params, unsigned int id):
	HasID (id),
	_part_manager (part_manager),
	_voice_manager (new VoiceManager (main_params, &_part_params, &_voice_params, work_performer)),
	_switch_wavetables (false),
	_wt_update_request (0),
	_wt_serial (0),
	_wt_wu (0),
	_wt_wu_ever_started (false),
	_ports (_part_manager->plugin(), this->id())
{
	_voice_manager->set_max_polyphony (64);

	_base_waves[0] = new DSP::ParametricWaves::Sine();
	_base_waves[1] = new DSP::ParametricWaves::Triangle();
	_base_waves[2] = new DSP::ParametricWaves::Square();
	_base_waves[3] = new DSP::ParametricWaves::Sawtooth();
	_base_waves[4] = new DSP::ParametricWaves::Pulse();
	_base_waves[5] = new DSP::ParametricWaves::Power();
	_base_waves[6] = new DSP::ParametricWaves::Gauss();
	_base_waves[7] = new DSP::ParametricWaves::Diode();
	_base_waves[8] = new DSP::ParametricWaves::Chirp();

	_modulator_waves[0] = new DSP::ParametricWaves::Sine();
	_modulator_waves[1] = new DSP::ParametricWaves::Triangle();
	_modulator_waves[2] = new DSP::ParametricWaves::Square();
	_modulator_waves[3] = new DSP::ParametricWaves::Sawtooth();

	// Double buffering of wavetables. The one with index 0 is always
	// the one currently used.
	_wavetables[0] = new DSP::Wavetable();
	_wavetables[1] = new DSP::Wavetable();
	_wt_wu = new UpdateWavetableWorkUnit (this);

	// Initially resize buffers:
	graph_updated();
	// Initially compute wavetable. Also makes it possible to wait
	// on work unit in the destructor:
	update_wavetable();

	// Listen on params change:
	_part_params.wave_shape.on_change.connect (this, &Part::update_wavetable);
	_part_params.modulator_amplitude.on_change.connect (this, &Part::update_wavetable);
	_part_params.modulator_index.on_change.connect (this, &Part::update_wavetable);
	_part_params.modulator_shape.on_change.connect (this, &Part::update_wavetable);
	//TODO rest of the non-polyphonic params
}


Part::~Part()
{
	// _wt_wu is never normally being waited on, so it's ok to wait here.
	// Must wait since it can still use Waves. It also needs to be deleted.
	_wt_wu->wait();

	for (size_t i = 0; i < ARRAY_SIZE (_base_waves); ++i)
		delete _base_waves[i];

	for (size_t i = 0; i < ARRAY_SIZE (_modulator_waves); ++i)
		delete _modulator_waves[i];

	delete _wt_wu;
	delete _voice_manager;
}


void
Part::handle_voice_event (Haruhi::VoiceEvent const* event)
{
	if (_part_params.part_enabled.get())
		_voice_manager->handle_voice_event (event);
}


void
Part::process_events()
{
	if (widget())
		widget()->process_events();
}


void
Part::panic()
{
	_voice_manager->panic();
}


void
Part::graph_updated()
{
	Haruhi::Graph* graph = _part_manager->graph();
	_voice_manager->graph_updated (graph->sample_rate(), graph->buffer_size());
}


void
Part::params_updated()
{
}


void
Part::update_wavetable()
{
	_wt_update_request.inc();
}


void
Part::check_wavetable_update_process()
{
	unsigned int update_request = _wt_update_request.load();

	if (update_request != _wt_serial.load())
	{
		if (!_wt_wu_ever_started || (_wt_wu->is_ready() && _wt_wu->serial() != update_request))
		{
			// Prepare work unit:
			_wt_wu->reset (_wavetables[1], update_request);
			_wt_wu_ever_started = true;

			Haruhi::Services::lo_priority_work_performer()->add (_wt_wu);
		}
	}
}


void
Part::render()
{
	if (_switch_wavetables.load())
	{
		std::swap (_wavetables[0], _wavetables[1]);
		_switch_wavetables.store (false);
		_voice_manager->set_wavetable (_wavetables[0]);
	}

	check_wavetable_update_process();

	_voice_manager->render();
}


void
Part::wait_for_render()
{
	_voice_manager->wait_for_render();
}


void
Part::mix_rendering_result (Haruhi::AudioBuffer* b1, Haruhi::AudioBuffer* b2)
{
	assert (b1 != 0);
	assert (b2 != 0);

	_voice_manager->mix_rendering_result (b1, b2);
}


unsigned int
Part::voices_number() const
{
	return _voice_manager->current_voices_number();
}


DSP::Wave*
Part::final_wave() const
{
	DSP::ParametricWave* bw = base_wave()->clone();
	DSP::ParametricWave* mw = modulator_wave()->clone();

	bw->set_param (_part_params.wave_shape.to_f());
	mw->set_param (_part_params.modulator_shape.to_f());

	// Add harmonics:
	DSP::HarmonicsWave* hw = new DSP::HarmonicsWave (bw, true);
	for (std::size_t i = 0; i < Params::Part::HarmonicsNumber; ++i)
	{
		float h = _part_params.harmonics[i].to_f();
		float p = _part_params.harmonic_phases[i].to_f();
		// Apply exponential curve to harmonic value:
		h = h > 0 ? FastPow::pow (h, M_E) : -FastPow::pow (-h, M_E);
		hw->set_harmonic (i, h, p);
	}

	// Add modulation:
	return new DSP::ModulatedWave (hw, mw, static_cast<DSP::ModulatedWave::Type> (_part_params.modulator_type.get()),
								   _part_params.modulator_amplitude.to_f(), _part_params.modulator_index.get(), true, true);
}


void
Part::wavetable_computed (unsigned int serial)
{
	_wt_serial.store (serial);
	_switch_wavetables.store (true);
}

} // namespace Yuki

