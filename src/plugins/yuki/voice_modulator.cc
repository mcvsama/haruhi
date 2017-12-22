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
#include <iterator>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wavetable.h>

// Local:
#include "voice_modulator.h"
#include "voice_operator.h"
#include "params.h"


namespace Yuki {

VoiceModulator::VoiceModulator (Params::Part* part_params, std::size_t buffer_size):
	_part_params (part_params),
	_buffer_size (buffer_size)
{
	assert (std::size (_operator_output) == std::size (_operator_fm_output));
	assert (std::size (_operator_output) == std::size (_operator));

	resize_buffers();
}


void
VoiceModulator::modulate (Haruhi::AudioBuffer* amplitude_buf_source, Haruhi::AudioBuffer* frequency_buf_source,
						  Haruhi::AudioBuffer* frequency_buf_target, Haruhi::AudioBuffer* tmp_bufs) noexcept
{
	frequency_buf_target->fill (1.0f);

	// tmp_bufs[0..Params::Part::OperatorsNumber] will be used as  FM buffers, the rest - as AM buffers.
	Haruhi::AudioBuffer* fm_bufs = tmp_bufs + 0;
	Haruhi::AudioBuffer* am_bufs = tmp_bufs + Params::Part::OperatorsNumber;

	for (std::size_t i = 0; i < Params::Part::OperatorsNumber; ++i)
	{
		fm_bufs[i].fill (frequency_buf_source);
		am_bufs[i].fill (1.0f);
	}

	// Mix matrix. Skip main output (last index):
	for (std::size_t o = 0; o < Params::Part::OperatorsNumber; ++o)
	{
		for (std::size_t i = 0; i < Params::Part::OperatorsNumber; ++i)
		{
			modulate_frequency (&fm_bufs[o], &_operator_fm_output[i], _part_params->fm_matrix[o][i].to_f());
			modulate_amplitude (&am_bufs[o], &_operator_output[i], _part_params->am_matrix[o][i].to_f());
		}
	}

	// Process operators:
	for (std::size_t o = 0; o < Params::Part::OperatorsNumber; ++o)
	{
		Params::Operator* p = &_part_params->operators[o];
		Haruhi::Sample detune = static_cast<Haruhi::Sample> (p->frequency_numerator) / p->frequency_denominator
			* FastPow::pow_radix_2 (p->octave.get() + (1.0f / 12.0f * p->detune.to_f()));
		_operator[o].set_frequency_source (&fm_bufs[o]);
		_operator[o].set_amplitude_source (&am_bufs[o]);
		_operator[o].set_detune (detune);
		_operator[o].fill (&_operator_output[o], &_operator_fm_output[o]);
	}

	// Modulate output buffers (mix operators to main oscillator):
	for (std::size_t i = 0; i < Params::Part::OperatorsNumber; ++i)
	{
		modulate_frequency (frequency_buf_target, &_operator_output[i], _part_params->fm_matrix[3][i].to_f());
		modulate_amplitude (amplitude_buf_source, &_operator_output[i], _part_params->am_matrix[3][i].to_f());
	}
}


void
VoiceModulator::graph_updated (std::size_t buffer_size)
{
	_buffer_size = buffer_size;

	resize_buffers();
}


void
VoiceModulator::set_oversampling (unsigned int oversampling)
{
	_oversampling = oversampling;

	resize_buffers();
}


void
VoiceModulator::resize_buffers()
{
	for (Haruhi::AudioBuffer& buf: _operator_output)
	{
		buf.resize (_buffer_size * _oversampling);
		buf.clear();
	}

	for (Haruhi::AudioBuffer& buf: _operator_fm_output)
	{
		buf.resize (_buffer_size * _oversampling);
		buf.clear();
	}
}


void
VoiceModulator::modulate_frequency (Haruhi::AudioBuffer* target, Haruhi::AudioBuffer* source, Haruhi::Sample factor) noexcept
{
	assert (target->size() == source->size());

	for (std::size_t i = 0; i < target->size(); ++i)
		(*target)[i] *= 1.0f + factor * (*source)[i];
}


void
VoiceModulator::modulate_amplitude (Haruhi::AudioBuffer* target, Haruhi::AudioBuffer* source, Haruhi::Sample factor) noexcept
{
	assert (target->size() == source->size());

	const float factor_sgn = factor;
	if (factor < 0.0f)
		factor = -factor;

	for (std::size_t i = 0; i < target->size(); ++i)
		(*target)[i] *= (1.0f - factor + factor_sgn * (*source)[i]);
}

} // namespace Yuki

