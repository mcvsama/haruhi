/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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
#include <haruhi/dsp/translated_wave.h>
#include <haruhi/dsp/scaled_wave.h>
#include <haruhi/utility/fast_pow.h>
#include <haruhi/utility/qdom.h>
#include <haruhi/utility/signal.h>

// Local:
#include "part.h"
#include "part_manager.h"
#include "plugin.h"
#include "voice_manager.h"


namespace Yuki {

Part::UpdateWavetableWorkUnit::UpdateWavetableWorkUnit (Part* part) noexcept:
	_part (part),
	_serial (0),
	_is_cancelled (false)
{ }


void
Part::UpdateWavetableWorkUnit::reset (DSP::Wavetable* wavetable, unsigned int serial) noexcept
{
	_wavetable = wavetable;
	_serial = serial;
	_is_cancelled.store (false);
}


void
Part::UpdateWavetableWorkUnit::execute()
{
	Unique<DSP::Wave> wave = _part->final_wave();

	DSP::FFTFiller filler (wave.get(), true, 0.000001f);
	filler.set_cancel_predicate (boost::bind (&UpdateWavetableWorkUnit::is_cancelled, this));
	filler.fill (_wavetable, 4096);

	if (!filler.was_interrupted())
	{
		// We're sure that Part still exists as long as this object exist,
		// because Part will wait for us in its destructor.
		_part->wavetable_computed (_serial);
	}
}


Part::PartPorts::PartPorts (Plugin* plugin, unsigned int part_id)
{
	port_group = std::make_unique<Haruhi::PortGroup> (plugin->graph(), QString ("Part %1").arg (part_id).toStdString());

	wave_shape					= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Wave shape", Haruhi::Port::Input, port_group.get());
	modulator_amplitude			= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Wave modulator amplitude", Haruhi::Port::Input, port_group.get());
	modulator_index				= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Wave modulator index", Haruhi::Port::Input, port_group.get());
	modulator_shape				= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Wave modulator shape", Haruhi::Port::Input, port_group.get());
	volume						= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Level", Haruhi::Port::Input, port_group.get());
	amplitude					= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Amplitude modulation", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	frequency					= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Frequency modulation", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	panorama					= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Panorama", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	detune						= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Detune", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	pitchbend					= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Pitchbend", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	velocity_sens				= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Velocity sensitivity", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	unison_index				= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Unison index", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	unison_spread				= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Unison spread", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	unison_init					= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Unison init. φ", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	unison_noise				= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Unison noise", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	unison_vibrato_level		= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Unison vibrato level", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	unison_vibrato_frequency	= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Unison vibrato frequency", Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	portamento_time				= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Portamento time", Haruhi::Port::Input, port_group.get());
	phase						= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Phase", Haruhi::Port::Input, port_group.get());
	noise_level					= std::make_unique<Haruhi::EventPort> (plugin, "Operator M - Noise level", Haruhi::Port::Input, port_group.get());

	for (unsigned int i = 0; i < 2; ++i)
	{
		filter_frequency[i]		= std::make_unique<Haruhi::EventPort> (plugin, QString ("Filter %1 - Frequency").arg (i + 1).toStdString(), Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
		filter_resonance[i]		= std::make_unique<Haruhi::EventPort> (plugin, QString ("Filter %1 - Resonance (Q)").arg (i + 1).toStdString(), Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
		filter_gain[i]			= std::make_unique<Haruhi::EventPort> (plugin, QString ("Filter %1 - Gain").arg (i + 1).toStdString(), Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
		filter_attenuation[i]	= std::make_unique<Haruhi::EventPort> (plugin, QString ("Filter %1 - Attenuation").arg (i + 1).toStdString(), Haruhi::Port::Input, port_group.get(), Haruhi::Port::Polyphonic);
	}

	for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
		operator_detune[i]		= std::make_unique<Haruhi::EventPort> (plugin, QString ("Operator %1 - Detune").arg (i + 1).toStdString(), Haruhi::Port::Input, port_group.get());

	fm_matrix.resize (Params::Part::OperatorsNumber + 1);
	am_matrix.resize (Params::Part::OperatorsNumber + 1);
	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
	{
		fm_matrix[o].resize (Params::Part::OperatorsNumber);
		am_matrix[o].resize (Params::Part::OperatorsNumber);
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
		{
			QString name = QString ("matrix [%1] → [%2]").arg (i);
			if (o == 0)
				name = name.arg ("M");
			else
				name = name.arg (o);
			fm_matrix[o][i] = std::make_unique<Haruhi::EventPort> (plugin, QString ("FM %1").arg (name).toStdString(), Haruhi::Port::Input, port_group.get());
			am_matrix[o][i] = std::make_unique<Haruhi::EventPort> (plugin, QString ("AM %1").arg (name).toStdString(), Haruhi::Port::Input, port_group.get());
		}
	}
}


Part::PartControllerProxies::PartControllerProxies (PartManager* part_manager, PartPorts* part_ports, Params::Part* part_params):
#define CONSTRUCT_CONTROLLER_PROXY(name) name (part_ports->name.get(), &part_params->name)
	CONSTRUCT_CONTROLLER_PROXY (volume),
	CONSTRUCT_CONTROLLER_PROXY (portamento_time),
	CONSTRUCT_CONTROLLER_PROXY (phase),
	CONSTRUCT_CONTROLLER_PROXY (noise_level),
	CONSTRUCT_CONTROLLER_PROXY (wave_shape),
	CONSTRUCT_CONTROLLER_PROXY (modulator_amplitude),
	CONSTRUCT_CONTROLLER_PROXY (modulator_index),
	CONSTRUCT_CONTROLLER_PROXY (modulator_shape),
#undef CONSTRUCT_CONTROLLER_PROXY

#define CONSTRUCT_CONTROLLER_PROXY(name) name (part_ports->name.get(), &part_params->voice.name)
	CONSTRUCT_CONTROLLER_PROXY (amplitude),
	CONSTRUCT_CONTROLLER_PROXY (frequency),
	CONSTRUCT_CONTROLLER_PROXY (panorama),
	CONSTRUCT_CONTROLLER_PROXY (detune),
	CONSTRUCT_CONTROLLER_PROXY (pitchbend),
	CONSTRUCT_CONTROLLER_PROXY (velocity_sens),
	CONSTRUCT_CONTROLLER_PROXY (unison_index),
	CONSTRUCT_CONTROLLER_PROXY (unison_spread),
	CONSTRUCT_CONTROLLER_PROXY (unison_init),
	CONSTRUCT_CONTROLLER_PROXY (unison_noise),
	CONSTRUCT_CONTROLLER_PROXY (unison_vibrato_level),
	CONSTRUCT_CONTROLLER_PROXY (unison_vibrato_frequency),
#undef CONSTRUCT_CONTROLLER_PROXY

	_part_manager (part_manager)
{
	// Filters:
	for (unsigned int i = 0; i < 2; ++i)
	{
		filter_frequency[i] = std::make_unique<Haruhi::ControllerProxy> (part_ports->filter_frequency[i].get(), &part_params->voice.filters[i].frequency);
		filter_resonance[i] = std::make_unique<Haruhi::ControllerProxy> (part_ports->filter_resonance[i].get(), &part_params->voice.filters[i].resonance);
		filter_gain[i] = std::make_unique<Haruhi::ControllerProxy> (part_ports->filter_gain[i].get(), &part_params->voice.filters[i].gain);
		filter_attenuation[i] = std::make_unique<Haruhi::ControllerProxy> (part_ports->filter_attenuation[i].get(), &part_params->voice.filters[i].attenuation);
	}

	// Operators:
	for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
		operator_detune[i] = std::make_unique<Haruhi::ControllerProxy> (part_ports->operator_detune[i].get(), &part_params->operators[i].detune);

	// Operator matrix:
	fm_matrix.resize (Params::Part::OperatorsNumber + 1);
	am_matrix.resize (Params::Part::OperatorsNumber + 1);
	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
	{
		fm_matrix[o].resize (Params::Part::OperatorsNumber);
		am_matrix[o].resize (Params::Part::OperatorsNumber);
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
		{
			fm_matrix[o][i] = std::make_unique<Haruhi::ControllerProxy> (part_ports->fm_matrix[o][i].get(), &part_params->fm_matrix[o][i]);
			am_matrix[o][i] = std::make_unique<Haruhi::ControllerProxy> (part_ports->am_matrix[o][i].get(), &part_params->am_matrix[o][i]);
		}
	}
}


void
Part::PartControllerProxies::process_events()
{
	// Forward all messages from common ports to parts' ports:
	forward_messages (_part_manager->ports()->amplitude.get(), amplitude.event_port());
	forward_messages (_part_manager->ports()->frequency.get(), frequency.event_port());
	forward_messages (_part_manager->ports()->pitchbend.get(), pitchbend.event_port());

#define PROXY_PROCESS_EVENTS(name) (name).process_events();
	PROXY_PROCESS_EVENTS (volume);
	PROXY_PROCESS_EVENTS (portamento_time);
	PROXY_PROCESS_EVENTS (phase);
	PROXY_PROCESS_EVENTS (noise_level);
	PROXY_PROCESS_EVENTS (wave_shape);
	PROXY_PROCESS_EVENTS (modulator_amplitude);
	PROXY_PROCESS_EVENTS (modulator_index);
	PROXY_PROCESS_EVENTS (modulator_shape);
	PROXY_PROCESS_EVENTS (amplitude);
	PROXY_PROCESS_EVENTS (frequency);
	PROXY_PROCESS_EVENTS (panorama);
	PROXY_PROCESS_EVENTS (detune);
	PROXY_PROCESS_EVENTS (pitchbend);
	PROXY_PROCESS_EVENTS (velocity_sens);
	PROXY_PROCESS_EVENTS (unison_index);
	PROXY_PROCESS_EVENTS (unison_spread);
	PROXY_PROCESS_EVENTS (unison_init);
	PROXY_PROCESS_EVENTS (unison_noise);
	PROXY_PROCESS_EVENTS (unison_vibrato_level);
	PROXY_PROCESS_EVENTS (unison_vibrato_frequency);
	PROXY_PROCESS_EVENTS (*filter_frequency[0]);
	PROXY_PROCESS_EVENTS (*filter_resonance[0]);
	PROXY_PROCESS_EVENTS (*filter_gain[0]);
	PROXY_PROCESS_EVENTS (*filter_attenuation[0]);
	PROXY_PROCESS_EVENTS (*filter_frequency[1]);
	PROXY_PROCESS_EVENTS (*filter_resonance[1]);
	PROXY_PROCESS_EVENTS (*filter_gain[1]);
	PROXY_PROCESS_EVENTS (*filter_attenuation[1]);
#undef PROXY_PROCESS_EVENTS

	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
	{
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
		{
			fm_matrix[o][i]->process_events();
			am_matrix[o][i]->process_events();
		}
	}
}


void
Part::PartControllerProxies::forward_messages (Haruhi::EventPort* source, Haruhi::EventPort* target)
{
	if (target->back_connections().empty())
	{
		if (source->back_connections().empty())
			target->event_buffer()->push (new Haruhi::ControllerEvent (Time::now(), source->default_value()));
		else
			target->event_buffer()->mixin (source->event_buffer());
	}
}


Part::ParamUpdaters::ParamUpdaters (VoiceManager* voice_manager):
#define CONSTRUCT_PARAM_UPDATER(name) name (voice_manager, &Params::Voice::name)
	CONSTRUCT_PARAM_UPDATER (amplitude),
	CONSTRUCT_PARAM_UPDATER (frequency),
	CONSTRUCT_PARAM_UPDATER (panorama),
	CONSTRUCT_PARAM_UPDATER (detune),
	CONSTRUCT_PARAM_UPDATER (pitchbend),
	CONSTRUCT_PARAM_UPDATER (velocity_sens),
	CONSTRUCT_PARAM_UPDATER (unison_index),
	CONSTRUCT_PARAM_UPDATER (unison_spread),
	CONSTRUCT_PARAM_UPDATER (unison_init),
	CONSTRUCT_PARAM_UPDATER (unison_noise),
	CONSTRUCT_PARAM_UPDATER (unison_vibrato_level),
	CONSTRUCT_PARAM_UPDATER (unison_vibrato_frequency)
#undef CONSTRUCT_PARAM_UPDATER
{
	for (unsigned int i = 0; i < 2; ++i)
	{
#define NEW_PARAM_UPDATER(name) filter_##name[i] = std::make_unique<FilterParamUpdater<Params::Filter::ControllerParamPtr>> (voice_manager, i, &Params::Filter::name)
		NEW_PARAM_UPDATER (frequency);
		NEW_PARAM_UPDATER (resonance);
		NEW_PARAM_UPDATER (gain);
		NEW_PARAM_UPDATER (attenuation);
#undef NEW_PARAM_UPDATER

#define NEW_PARAM_UPDATER(name) filter_##name[i] = std::make_unique<FilterParamUpdater<Params::Filter::IntParamPtr>> (voice_manager, i, &Params::Filter::name)
		NEW_PARAM_UPDATER (enabled);
		NEW_PARAM_UPDATER (type);
		NEW_PARAM_UPDATER (stages);
		NEW_PARAM_UPDATER (limiter_enabled);
#undef NEW_PARAM_UPDATER
	}
}


Part::Part (PartManager* part_manager, WorkPerformer* work_performer, Params::Main* main_params, unsigned int id):
	HasID (id),
	HasPlugin (part_manager->plugin()),
	_part_manager (part_manager),
	_voice_manager (std::make_unique<VoiceManager> (main_params, &_part_params, work_performer)),
	_ports (_part_manager->plugin(), this->id()),
	_proxies (_part_manager, &_ports, &_part_params),
	_updaters (_voice_manager.get())
{
	_base_waves[0] = std::make_unique<DSP::ParametricWaves::Sine>();
	_base_waves[1] = std::make_unique<DSP::ParametricWaves::Triangle>();
	_base_waves[2] = std::make_unique<DSP::ParametricWaves::Square>();
	_base_waves[3] = std::make_unique<DSP::ParametricWaves::Sawtooth>();
	_base_waves[4] = std::make_unique<DSP::ParametricWaves::Pulse>();
	_base_waves[5] = std::make_unique<DSP::ParametricWaves::Power>();
	_base_waves[6] = std::make_unique<DSP::ParametricWaves::Gauss>();
	_base_waves[7] = std::make_unique<DSP::ParametricWaves::Diode>();
	_base_waves[8] = std::make_unique<DSP::ParametricWaves::Chirp>();

	_modulator_waves[0] = std::make_unique<DSP::ParametricWaves::Sine>();
	_modulator_waves[1] = std::make_unique<DSP::ParametricWaves::Triangle>();
	_modulator_waves[2] = std::make_unique<DSP::ParametricWaves::Square>();
	_modulator_waves[3] = std::make_unique<DSP::ParametricWaves::Sawtooth>();

	_wave = &_wave_current;

	_wt_wu = std::make_unique<UpdateWavetableWorkUnit> (this);

	// Initially resize buffers:
	graph_updated();
	// Initially compute wavetable. Also makes it possible to wait
	// on work unit in the destructor:
	update_wavetable();

#define UPDATE_WAVETABLE_ON_CHANGE(name) _part_params.name.on_change.connect (this, &Part::update_wavetable)
	// Listen on params change. Only on params that need additional
	// action when they change (eg. updating wavetable).
	UPDATE_WAVETABLE_ON_CHANGE (wave_type);
	UPDATE_WAVETABLE_ON_CHANGE (modulator_wave_type);
	UPDATE_WAVETABLE_ON_CHANGE (modulator_type);
	UPDATE_WAVETABLE_ON_CHANGE (wave_shape);
	UPDATE_WAVETABLE_ON_CHANGE (modulator_amplitude);
	UPDATE_WAVETABLE_ON_CHANGE (modulator_index);
	UPDATE_WAVETABLE_ON_CHANGE (modulator_shape);
	UPDATE_WAVETABLE_ON_CHANGE (auto_center);
	for (std::size_t i = 0; i < countof (_part_params.harmonics); ++i)
		UPDATE_WAVETABLE_ON_CHANGE (harmonics[i]);
	for (std::size_t i = 0; i < countof (_part_params.harmonic_phases); ++i)
		UPDATE_WAVETABLE_ON_CHANGE (harmonic_phases[i]);
#undef UPDATE_WAVETABLE_ON_CHANGE

#define UPDATE_VOICES_ON_VCE(name) \
	_proxies.name.on_voice_controller_event.connect (&_updaters.name, &VoiceParamUpdater<Params::Voice::ControllerParamPtr>::handle_event); \
	_part_params.voice.name.on_change_with_value.connect (&_updaters.name, &VoiceParamUpdater<Params::Voice::ControllerParamPtr>::handle_change)
	// Updaters for Voice params:
	UPDATE_VOICES_ON_VCE (amplitude);
	UPDATE_VOICES_ON_VCE (frequency);
	UPDATE_VOICES_ON_VCE (panorama);
	UPDATE_VOICES_ON_VCE (detune);
	UPDATE_VOICES_ON_VCE (pitchbend);
	UPDATE_VOICES_ON_VCE (velocity_sens);
	UPDATE_VOICES_ON_VCE (unison_index);
	UPDATE_VOICES_ON_VCE (unison_spread);
	UPDATE_VOICES_ON_VCE (unison_init);
	UPDATE_VOICES_ON_VCE (unison_noise);
	UPDATE_VOICES_ON_VCE (unison_vibrato_level);
	UPDATE_VOICES_ON_VCE (unison_vibrato_frequency);
#undef UPDATE_VOICES_ON_VCE

#define UPDATE_FILTERS_ON_VCE(name) \
	_proxies.filter_##name[0]->on_voice_controller_event.connect (_updaters.filter_##name[0].get(), &FilterParamUpdater<Params::Filter::ControllerParamPtr>::handle_event); \
	_part_params.voice.filters[0].name.on_change_with_value.connect (_updaters.filter_##name[0].get(), &FilterParamUpdater<Params::Filter::ControllerParamPtr>::handle_change);
	// Updaters for Filter 1 params:
	UPDATE_FILTERS_ON_VCE (frequency);
	UPDATE_FILTERS_ON_VCE (resonance);
	UPDATE_FILTERS_ON_VCE (gain);
	UPDATE_FILTERS_ON_VCE (attenuation);
#undef UPDATE_FILTERS_ON_VCE

#define UPDATE_FILTERS_ON_VCE(name) \
	_proxies.filter_##name[1]->on_voice_controller_event.connect (_updaters.filter_##name[1].get(), &FilterParamUpdater<Params::Filter::ControllerParamPtr>::handle_event); \
	_part_params.voice.filters[1].name.on_change_with_value.connect (_updaters.filter_##name[1].get(), &FilterParamUpdater<Params::Filter::ControllerParamPtr>::handle_change);
	// Updaters for Filter 2 params:
	UPDATE_FILTERS_ON_VCE (frequency);
	UPDATE_FILTERS_ON_VCE (resonance);
	UPDATE_FILTERS_ON_VCE (gain);
	UPDATE_FILTERS_ON_VCE (attenuation);
#undef UPDATE_FILTERS_ON_VCE

#define UPDATE_FILTERS_ON_CHANGE(name) \
	_part_params.voice.filters[0].name.on_change_with_value.connect (_updaters.filter_##name[0].get(), &FilterParamUpdater<Params::Filter::IntParamPtr>::handle_change);
	// Updaters for Filter 1 params:
	UPDATE_FILTERS_ON_CHANGE (enabled);
	UPDATE_FILTERS_ON_CHANGE (type);
	UPDATE_FILTERS_ON_CHANGE (stages);
	UPDATE_FILTERS_ON_CHANGE (limiter_enabled);
#undef UPDATE_FILTERS_ON_CHANGE

#define UPDATE_FILTERS_ON_CHANGE(name) \
	_part_params.voice.filters[1].name.on_change_with_value.connect (_updaters.filter_##name[1].get(), &FilterParamUpdater<Params::Filter::IntParamPtr>::handle_change);
	// Updaters for Filter 2 params:
	UPDATE_FILTERS_ON_CHANGE (enabled);
	UPDATE_FILTERS_ON_CHANGE (type);
	UPDATE_FILTERS_ON_CHANGE (stages);
	UPDATE_FILTERS_ON_CHANGE (limiter_enabled);
#undef UPDATE_FILTERS_ON_CHANGE
}


Part::~Part()
{
	// Disconnect listeners way before proxies and part are destroyed.
	Signal::Receiver::disconnect_all_signals();

	// Need to wait for _wt_wu, since it uses a pointer to our member.
	// _wt_wu is never normally being waited on, so it's ok to wait here.
	// Must wait since it can still use Waves. It also needs to be deleted.
	_wt_wu->wait();
}


void
Part::handle_voice_event (Haruhi::VoiceEvent const* event)
{
	if (_part_params.part_enabled.get())
		_voice_manager->handle_voice_event (event);
}


void
Part::handle_amplitude_event (Haruhi::VoiceControllerEvent const* event)
{
	if (_part_params.part_enabled.get())
		_voice_manager->handle_amplitude_event (event);
}


void
Part::handle_frequency_event (Haruhi::VoiceControllerEvent const* event)
{
	if (_part_params.part_enabled.get())
		_voice_manager->handle_frequency_event (event);
}


void
Part::process_events()
{
	_proxies.process_events();
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
Part::set_oversampling (unsigned int oversampling)
{
	_voice_manager->set_oversampling (oversampling);
}


void
Part::update_wavetable()
{
	++_wt_update_request;
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
			_wt_wu->reset (&_wavetable_rendered, update_request);
			_wt_wu_ever_started = true;

			Haruhi::Services::lo_priority_work_performer()->add (_wt_wu.get());
		}
	}
}


void
Part::async_render()
{
	using std::swap;

	auto switch_next_to_current_wavetable = [&] {
		swap (_wavetable_next, _wavetable_current);

		if (_wavetable_current.computed())
			_wave = &_wave_current;
		else
			_wave = nullptr;

		_voice_manager->set_wave (_wave);
	};

	// If crossing-wave is not running, this has no effect.
	// Otherwise it works like advancing on the next rendering round.
	_crossing_wave.advance (_part_manager->graph()->buffer_size());

	switch (_crossing_wave.state())
	{
		case DSP::CrossingWave::NotStarted:
			if (_new_wavetable_ready.load())
			{
				_new_wavetable_ready.store (false);

				swap (_wavetable_rendered, _wavetable_next);

				// Ensure it's not the first computation of wavetable if crossing-wave is to be used:
				if (_wavetable_current.computed() && _wavetable_next.computed())
				{
					// Time choosen by ear:
					auto transition_time = 10_ms;
					auto transition_samples = transition_time * _part_manager->graph()->sample_rate();
					_crossing_wave = DSP::CrossingWave (&_wave_current, &_wave_next, transition_samples);
					_crossing_wave.start();

					_wave = &_crossing_wave;
					_voice_manager->set_wave (_wave);
				}
				else
					switch_next_to_current_wavetable();
			}
			break;

		case DSP::CrossingWave::Running:
			break;

		case DSP::CrossingWave::Finished:
			switch_next_to_current_wavetable();

			// Switch state to NotStarted:
			_crossing_wave.reset();
			break;
	}

	if (_crossing_wave.state() == DSP::CrossingWave::NotStarted)
		check_wavetable_update_process();

	_voice_manager->async_render();
}


void
Part::wait_for_render()
{
	_voice_manager->wait_for_render();
}


void
Part::mix_rendering_result (Haruhi::AudioBuffer* b1, Haruhi::AudioBuffer* b2) noexcept
{
	assert (b1 != 0);
	assert (b2 != 0);

	_voice_manager->mix_rendering_result (b1, b2);

	if (_part_params.pseudo_stereo.get())
		b2->negate();
}


unsigned int
Part::voices_number() const
{
	return _voice_manager->current_voices_number();
}


Unique<DSP::Wave>
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

	Unique<DSP::Wave> final = std::make_unique<DSP::ModulatedWave> (hw, mw, static_cast<DSP::ModulatedWave::Type> (_part_params.modulator_type.get()),
																	_part_params.modulator_amplitude.to_f(), _part_params.modulator_index.get(), true, true);

	if (_part_params.auto_center)
	{
		std::pair<Sample, Sample> min_max = final->compute_min_max();
		Sample translation = -0.5f * (min_max.second + min_max.first);
		final = std::make_unique<DSP::TranslatedWave> (translation, final.release(), true);
	}

	return final;
}


void
Part::save_state (QDomElement& element) const
{
	{
		QDomElement e = element.ownerDocument().createElement ("part-parameters");
		_part_params.save_state (e);
		element.appendChild (e);
	}

	for (std::size_t i = 0; i < Params::Part::OperatorsNumber; ++i)
	{
		QDomElement e = element.ownerDocument().createElement (QString ("operator-%1-parameters").arg (i + 1));
		_part_params.operators[i].save_state (e);
		element.appendChild (e);
	}

	{
		QDomElement e = element.ownerDocument().createElement ("voice-parameters");
		_part_params.voice.save_state (e);
		element.appendChild (e);
	}

	for (std::size_t i = 0; i < Params::Voice::FiltersNumber; ++i)
	{
		QDomElement e = element.ownerDocument().createElement (QString ("filter-%1-parameters").arg (i + 1));
		_part_params.voice.filters[i].save_state (e);
		element.appendChild (e);
	}
}


void
Part::load_state (QDomElement const& element)
{
	for (QDomElement& e: element)
	{
		if (e.tagName() == "part-parameters")
			_part_params.load_state (e);
		else if (e.tagName() == "operator-1-parameters")
			_part_params.operators[0].load_state (e);
		else if (e.tagName() == "operator-2-parameters")
			_part_params.operators[1].load_state (e);
		else if (e.tagName() == "operator-3-parameters")
			_part_params.operators[2].load_state (e);
		else if (e.tagName() == "voice-parameters")
			_part_params.voice.load_state (e);
		else if (e.tagName() == "filter-1-parameters")
			_part_params.voice.filters[0].load_state (e);
		else if (e.tagName() == "filter-2-parameters")
			_part_params.voice.filters[1].load_state (e);
	}
}


void
Part::wavetable_computed (unsigned int serial)
{
	_wt_serial.store (serial);
	_new_wavetable_ready.store (true);
}

} // namespace Yuki

