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

// Local:
#include "part.h"
#include "part_manager.h"
#include "plugin.h"
#include "voice_manager.h"


namespace Yuki {

Part::UpdateWavetableWorkUnit::UpdateWavetableWorkUnit (Part* part):
	_part (part),
	_wave (0),
	_wavetable (0),
	_serial (0),
	_is_cancelled (false)
{ }


void
Part::UpdateWavetableWorkUnit::reset (DSP::Wave* wave, DSP::Wavetable* wavetable, unsigned int serial)
{
	_wave = wave;
	_wavetable = wavetable;
	_serial = serial;
	_is_cancelled.store (false);
}


void
Part::UpdateWavetableWorkUnit::execute()
{
	DSP::FFTFiller filler (_wave, true);
	filler.set_cancel_predicate (boost::bind (&UpdateWavetableWorkUnit::is_cancelled, this));
	filler.fill (_wavetable, 4096);

	if (!filler.was_interrupted())
	{
		// We're sure that Part still exists as long as this object exist,
		// because Part will wait for us in its destructor.
		_part->switch_wavetables();
	}
}


Part::Part (PartManager* part_manager, WorkPerformer* work_performer, Params::Main* main_params):
	_part_manager (part_manager),
	_voice_manager (new VoiceManager (main_params, &_part_params, &_voice_params, work_performer)),
	_switch_wavetables (false),
	_wt_update_request (0),
	_wt_serial (0),
	_wt_wu (0),
	_wt_wu_ever_started (false)
{
	_voice_manager->set_max_polyphony (64);

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
}


Part::~Part()
{
	// _wt_wu is never normally being waited on, so it's ok to wait here.
	_wt_wu->wait();

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
Part::process()
{
	// TODO process controllers
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


// TODO detect that parameter has changed and call this update_wavetable()
void
Part::update_wavetable()
{
	_wt_update_request.inc();
	check_wavetable_update_process();
}


void
Part::check_wavetable_update_process()
{
	unsigned int update_request = _wt_update_request.load();

	if (update_request != _wt_serial.load())
	{
		if (!_wt_wu_ever_started || (_wt_wu->is_ready() && _wt_wu->serial() != update_request))
		{
			// TODO wave
			_wt_wu->reset (new DSP::ParametricWaves::Sine(), _wavetables[1], update_request);
			_wt_wu_ever_started = true;
			Haruhi::Services::lo_priority_work_performer()->add (_wt_wu);
		}
		else if (_wt_wu->serial() != update_request)
			_wt_wu->cancel();
	}
}


void
Part::render()
{
	check_wavetable_update_process();

	if (_switch_wavetables.load())
	{
		std::swap (_wavetables[0], _wavetables[1]);
		_switch_wavetables.store (false);
		_voice_manager->set_wavetable (_wavetables[0]);
	}

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


void
Part::switch_wavetables()
{
	_wt_serial.inc();
	_switch_wavetables.store (true);
}

} // namespace Yuki

