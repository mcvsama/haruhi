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

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "voice.h"


namespace Yuki {

void
Voice::SharedResources::graph_updated (unsigned int, std::size_t buffer_size)
{
	amplitude_buf.resize (buffer_size);
	frequency_buf.resize (buffer_size);
}


Voice::Voice (Haruhi::VoiceID id, Haruhi::Timestamp timestamp, Params::Part* part_params, Sample amplitude, Sample frequency, unsigned int sample_rate, std::size_t buffer_size):
	_id (id),
	_timestamp (timestamp),
	_state (NotStarted),
	_part_params (part_params),
	_amplitude (amplitude),
	_frequency (frequency),
	_sample_rate (sample_rate),
	_buffer_size (buffer_size)
{
	_state = Voicing;

	graph_updated (sample_rate, buffer_size);

}


bool
Voice::render (SharedResources* res)
{
	// Setup VoiceOscillator:
	_vosc.set_amplitude_source (&res->amplitude_buf);
	_vosc.set_frequency_source (&res->frequency_buf);
	_vosc.set_phase (_part_params->phase.to_f());
	_vosc.set_initial_phases_spread (_params.unison_init.to_f());

	// Setup smoothers for 5ms/50ms. Response time must be independent from sample rate.
	_smoother_amplitude.set_samples (0.005f * _sample_rate);
	_smoother_frequency.set_samples (0.005f * _sample_rate);
	_smoother_pitchbend.set_samples (0.05f * _sample_rate);
	_smoother_panorama_1.set_samples (0.005f * _sample_rate);
	_smoother_panorama_2.set_samples (0.005f * _sample_rate);

	// TODO
	return false;
}


void
Voice::graph_updated (unsigned int sample_rate, std::size_t buffer_size)
{
	_sample_rate = sample_rate;
	_buffer_size = buffer_size;

	_output_1.resize (buffer_size);
	_output_2.resize (buffer_size);
}


void
Voice::set_wavetable (DSP::Wavetable* wavetable)
{
	_vosc.set_wavetable (wavetable);
}

} // namespace Yuki

