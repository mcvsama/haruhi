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

// Qt:
#include <QtGui/QMessageBox>

// Haruhi:
#include <haruhi/graph/conn_set.h>

// Local:
#include "mikuru.h"
#include "oscillator.h"
#include "filter.h"
#include "params.h"
#include "patch.h"
#include "envelopes.h"
#include "common_filters.h"
#include "part_effects.h"
#include "part_filters.h"
#include "effects/waveshaper.h"
#include "envelopes/adsr.h"
#include "envelopes/eg.h"
#include "envelopes/lfo.h"


namespace MikuruPrivate {

const char* Patch::VERSION_D_2010_02_20 = "D2010-02-20";


Patch::Patch (Mikuru* mikuru):
	_mikuru (mikuru)
{
}


void
Patch::save_state (QDomElement& element) const
{
	element.setAttribute ("version", Patch::VERSION_D_2010_02_20);

	// General:
	{
		QDomElement general_element = element.ownerDocument().createElement ("general");

		General* general = _mikuru->general();
		Params::General params (*general->params());

		save_parameter (general_element, "polyphony", params.polyphony);
		save_parameter (general_element, "enable-audio-input", params.enable_audio_input);
		save_parameter (general_element, "volume", general->_knob_volume);
		save_parameter (general_element, "detune", general->_knob_detune);
		save_parameter (general_element, "panorama", general->_knob_panorama);
		save_parameter (general_element, "stereo-width", general->_knob_stereo_width);
		save_parameter (general_element, "input-volume", general->_knob_input_volume);

		element.appendChild (general_element);
	}

	// Common filters:
	{
		QDomElement common_filters_element = element.ownerDocument().createElement ("common-filters");

		CommonFilters* common_filters = _mikuru->common_filters();
		Params::CommonFilters params (*common_filters->params());

		QDomElement filter1_element = element.ownerDocument().createElement ("filter1");
		QDomElement filter2_element = element.ownerDocument().createElement ("filter2");

		save_parameter (common_filters_element, "filter-configuration", params.filter_configuration);
		save_parameter (common_filters_element, "route-audio-input", params.route_audio_input);

		// Filter1:
		{
			Filter* filter = common_filters->filter1();
			Params::Filter params (*filter->params());

			save_parameter (filter1_element, "enabled", params.enabled);
			save_parameter (filter1_element, "type", params.type);
			save_parameter (filter1_element, "passes", params.passes);
			save_parameter (filter1_element, "limiter-enabled", params.limiter_enabled);
			save_parameter (filter1_element, "frequency", filter->_knob_frequency);
			save_parameter (filter1_element, "resonance", filter->_knob_resonance);
			save_parameter (filter1_element, "gain", filter->_knob_gain);
			save_parameter (filter1_element, "attenuation", filter->_knob_attenuation);
		}

		// Filter 2:
		{
			Filter* filter = common_filters->filter2();
			Params::Filter params (*filter->params());

			save_parameter (filter2_element, "enabled", params.enabled);
			save_parameter (filter2_element, "type", params.type);
			save_parameter (filter2_element, "passes", params.passes);
			save_parameter (filter2_element, "limiter-enabled", params.limiter_enabled);
			save_parameter (filter2_element, "frequency", filter->_knob_frequency);
			save_parameter (filter2_element, "resonance", filter->_knob_resonance);
			save_parameter (filter2_element, "gain", filter->_knob_gain);
			save_parameter (filter2_element, "attenuation", filter->_knob_attenuation);
		}

		common_filters_element.appendChild (filter1_element);
		common_filters_element.appendChild (filter2_element);
		element.appendChild (common_filters_element);
	}

	// Parts sorted by their tab-position:
	std::multimap<int, QDomElement> sorted_parts;
	for (Mikuru::Parts::iterator pt = _mikuru->parts().begin(); pt != _mikuru->parts().end(); ++pt)
	{
		QDomElement part_element = element.ownerDocument().createElement ("part");
		QDomElement oscillator_element = element.ownerDocument().createElement ("oscillator");
		QDomElement filters_element = element.ownerDocument().createElement ("filters");

		// General:
		{
			Params::Part params (*(*pt)->params());

			save_parameter (part_element, "enabled", params.enabled);
		}

		// Oscillator:
		{
			Oscillator* oscillator = (*pt)->oscillator();
			Params::Waveform waveform_params (*oscillator->waveform_params());
			Params::Oscillator oscillator_params (*oscillator->oscillator_params());
			Params::Voice voice_params (*oscillator->voice_params());

			// Knobs&other:
			save_parameter (oscillator_element, "wave-type", waveform_params.wave_type);
			save_parameter (oscillator_element, "wave-shape", waveform_params.wave_shape);
			save_parameter (oscillator_element, "modulator-type", waveform_params.modulator_type);
			save_parameter (oscillator_element, "modulator-wave-type", waveform_params.modulator_wave_type);
			save_parameter (oscillator_element, "modulator-amplitude", waveform_params.modulator_amplitude);
			save_parameter (oscillator_element, "modulator-index", waveform_params.modulator_index);
			save_parameter (oscillator_element, "modulator-shape", waveform_params.modulator_shape);
			for (Oscillator::Sliders::size_type i = 0; i < Params::Waveform::HarmonicsNumber; ++i)
				save_parameter (oscillator_element, QString ("harmonic-%1").arg (i), waveform_params.harmonics[i]);
			for (Oscillator::Sliders::size_type i = 0; i < Params::Waveform::HarmonicsNumber; ++i)
				save_parameter (oscillator_element, QString ("phase-%1").arg (i), waveform_params.phases[i]);

			// Knobs:
			save_parameter (oscillator_element, "volume", oscillator->_knob_volume);
			save_parameter (oscillator_element, "portamento-time", oscillator->_knob_portamento_time);
			save_parameter (oscillator_element, "phase", oscillator->_knob_phase);
			save_parameter (oscillator_element, "noise-level", oscillator->_knob_noise_level);
			// Other:
			save_parameter (oscillator_element, "wave-enabled", oscillator_params.wave_enabled);
			save_parameter (oscillator_element, "noise-enabled", oscillator_params.noise_enabled);
			save_parameter (oscillator_element, "frequency-modulation-range", oscillator_params.frequency_mod_range);
			save_parameter (oscillator_element, "pitchbend-enabled", oscillator_params.pitchbend_enabled);
			save_parameter (oscillator_element, "pitchbend-released", oscillator_params.pitchbend_released);
			save_parameter (oscillator_element, "pitchbend-up-semitones", oscillator_params.pitchbend_up_semitones);
			save_parameter (oscillator_element, "pitchbend-down-semitones", oscillator_params.pitchbend_down_semitones);
			save_parameter (oscillator_element, "transposition-semitones", oscillator_params.transposition_semitones);
			save_parameter (oscillator_element, "monophonic", oscillator_params.monophonic);
			save_parameter (oscillator_element, "monophonic-retrigger", oscillator_params.monophonic_retrigger);
			save_parameter (oscillator_element, "monophonic-key-priority", oscillator_params.monophonic_key_priority);
			save_parameter (oscillator_element, "const-portamento-time", oscillator_params.const_portamento_time);

			// Knobs:
			save_parameter (oscillator_element, "amplitude", voice_params.amplitude);
			save_parameter (oscillator_element, "frequency", voice_params.frequency);
			save_parameter (oscillator_element, "panorama", oscillator->_knob_panorama);
			save_parameter (oscillator_element, "detune", oscillator->_knob_detune);
			save_parameter (oscillator_element, "pitchbend", oscillator->_knob_pitchbend);
			save_parameter (oscillator_element, "velocity-sens", oscillator->_knob_velocity_sens);
			save_parameter (oscillator_element, "unison-index", oscillator->_knob_unison_index);
			save_parameter (oscillator_element, "unison-spread", oscillator->_knob_unison_spread);
			save_parameter (oscillator_element, "unison-init", oscillator->_knob_unison_init);
			save_parameter (oscillator_element, "unison-noise", oscillator->_knob_unison_noise);
		}

		// Part filters:
		{
			PartFilters* part_filters = (*pt)->filters();
			Params::PartFilters params (*part_filters->params());

			QDomElement filter1_element = element.ownerDocument().createElement ("filter1");
			QDomElement filter2_element = element.ownerDocument().createElement ("filter2");

			save_parameter (filters_element, "filter-configuration", params.filter_configuration);

			// Filter1:
			{
				Filter* filter = part_filters->filter1();
				Params::Filter params (*filter->params());

				save_parameter (filter1_element, "enabled", params.enabled);
				save_parameter (filter1_element, "type", params.type);
				save_parameter (filter1_element, "passes", params.passes);
				save_parameter (filter1_element, "limiter-enabled", params.limiter_enabled);
				save_parameter (filter1_element, "frequency", filter->_knob_frequency);
				save_parameter (filter1_element, "resonance", filter->_knob_resonance);
				save_parameter (filter1_element, "gain", filter->_knob_gain);
				save_parameter (filter1_element, "attenuation", filter->_knob_attenuation);
			}

			// Filter 2:
			{
				Filter* filter = part_filters->filter2();
				Params::Filter params (*filter->params());

				save_parameter (filter2_element, "enabled", params.enabled);
				save_parameter (filter2_element, "type", params.type);
				save_parameter (filter2_element, "passes", params.passes);
				save_parameter (filter2_element, "limiter-enabled", params.limiter_enabled);
				save_parameter (filter2_element, "frequency", filter->_knob_frequency);
				save_parameter (filter2_element, "resonance", filter->_knob_resonance);
				save_parameter (filter2_element, "gain", filter->_knob_gain);
				save_parameter (filter2_element, "attenuation", filter->_knob_attenuation);
			}

			filters_element.appendChild (filter1_element);
			filters_element.appendChild (filter2_element);
		}

		// Part effects sorted by their tab-position:
		std::multimap<int, QDomElement> sorted_effects;
		(*pt)->effects()->effects_mutex().lock();
		{
			PartEffects* part_effects = (*pt)->effects();
			for (PartEffects::EffectsList::iterator ef = part_effects->effects().begin(); ef != part_effects->effects().end(); ++ef)
			{
				QDomElement effect_element = element.ownerDocument().createElement ("effect");
				Waveshaper* waveshaper;

				if ((waveshaper = dynamic_cast<Waveshaper*> (*ef)))
				{
					Params::Waveshaper params (*waveshaper->params());

					effect_element.setAttribute ("type", "waveshaper");
					effect_element.setAttribute ("id", waveshaper->id());
					// Knobs:
					save_parameter (effect_element, "gain", waveshaper->_knob_gain);
					save_parameter (effect_element, "parameter", waveshaper->_knob_parameter);
					// Other:
					save_parameter (effect_element, "enabled", params.enabled);
					save_parameter (effect_element, "type", params.type);
				}

				// Tab position:
				sorted_effects.insert (std::make_pair ((*pt)->effects()->effect_tab_position (*ef), effect_element));
			}
		}
		(*pt)->effects()->effects_mutex().unlock();

		part_element.appendChild (oscillator_element);
		part_element.appendChild (filters_element);
		for (std::multimap<int, QDomElement>::iterator e = sorted_effects.begin(); e != sorted_effects.end(); ++e)
			part_element.appendChild (e->second);
		// Tab position:
		sorted_parts.insert (std::make_pair (_mikuru->part_tab_position (*pt), part_element));
	}
	for (std::multimap<int, QDomElement>::iterator p = sorted_parts.begin(); p != sorted_parts.end(); ++p)
		element.appendChild (p->second);

	// Envelopes sorted by their tab-position:
	std::multimap<int, QDomElement> sorted_envelopes;
	_mikuru->general()->envelopes()->envelopes_mutex().lock();
	for (Envelopes::EnvelopesList::iterator en = _mikuru->general()->envelopes()->envelopes().begin(); en != _mikuru->general()->envelopes()->envelopes().end(); ++en)
	{
		QDomElement envelope_element = element.ownerDocument().createElement ("envelope");
		ADSR* adsr;
		EG* eg;
		LFO* lfo;

		if ((adsr = dynamic_cast<ADSR*> (*en)))
		{
			Params::ADSR params (*adsr->params());

			envelope_element.setAttribute ("type", "adsr");
			envelope_element.setAttribute ("id", adsr->id());
			// Knobs:
			save_parameter (envelope_element, "delay", adsr->_knob_delay);
			save_parameter (envelope_element, "attack", adsr->_knob_attack);
			save_parameter (envelope_element, "decay", adsr->_knob_decay);
			save_parameter (envelope_element, "sustain", adsr->_knob_sustain);
			save_parameter (envelope_element, "release", adsr->_knob_release);
			// Other:
			save_parameter (envelope_element, "enabled", params.enabled);
			save_parameter (envelope_element, "direct-adsr", params.direct_adsr);
			save_parameter (envelope_element, "forced-release", params.forced_release);
			save_parameter (envelope_element, "sustain-enabled", params.sustain_enabled);
			save_parameter (envelope_element, "function", params.function);
			save_parameter (envelope_element, "mode", params.mode);
		}
		else if ((eg = dynamic_cast<EG*> (*en)))
		{
			Params::EG params (*eg->params());

			envelope_element.setAttribute ("type", "eg");
			envelope_element.setAttribute ("id", eg->id());
			// Other:
			save_parameter (envelope_element, "enabled", params.enabled);
			save_parameter (envelope_element, "segments", params.segments);
			save_parameter (envelope_element, "sustain-point", params.sustain_point);
			// Save envelope points:
			QDomElement points_element = envelope_element.ownerDocument().createElement ("points");
			envelope_element.appendChild (points_element);
			for (size_t i = 0; i < params.segments + 1; ++i)
			{
				QDomElement point_element = points_element.ownerDocument().createElement ("point");
				point_element.setAttribute ("value", params.values[i]);
				point_element.setAttribute ("duration", params.durations[i]);
				points_element.appendChild (point_element);
			}
		}
		else if ((lfo = dynamic_cast<LFO*> (*en)))
		{
			Params::LFO params (*lfo->params());

			envelope_element.setAttribute ("type", "lfo");
			envelope_element.setAttribute ("id", lfo->id());
			// Knobs:
			save_parameter (envelope_element, "delay", lfo->_knob_delay);
			save_parameter (envelope_element, "fade-in", lfo->_knob_fade_in);
			save_parameter (envelope_element, "frequency", lfo->_knob_frequency);
			save_parameter (envelope_element, "level", lfo->_knob_level);
			save_parameter (envelope_element, "depth", lfo->_knob_depth);
			save_parameter (envelope_element, "phase", lfo->_knob_phase);
			save_parameter (envelope_element, "wave-shape", lfo->_knob_wave_shape);
			save_parameter (envelope_element, "fade-out", lfo->_knob_fade_out);
			// Other:
			save_parameter (envelope_element, "enabled", params.enabled);
			save_parameter (envelope_element, "wave-type", params.wave_type);
			save_parameter (envelope_element, "wave-invert", params.wave_invert);
			save_parameter (envelope_element, "function", params.function);
			save_parameter (envelope_element, "mode", params.mode);
			save_parameter (envelope_element, "tempo-sync", params.tempo_sync);
			save_parameter (envelope_element, "tempo-numerator", params.tempo_numerator);
			save_parameter (envelope_element, "tempo-denominator", params.tempo_denominator);
			save_parameter (envelope_element, "random-start-phase", params.random_start_phase);
			save_parameter (envelope_element, "fade-out-enabled", params.fade_out_enabled);
		}

		// Tab position:
		sorted_envelopes.insert (std::make_pair (_mikuru->general()->envelopes()->envelope_tab_position (*en), envelope_element));
	}
	_mikuru->general()->envelopes()->envelopes_mutex().unlock();
	for (std::multimap<int, QDomElement>::iterator e = sorted_envelopes.begin(); e != sorted_envelopes.end(); ++e)
		element.appendChild (e->second);

	// Connections:
	{
		QDomElement connections_element = element.ownerDocument().createElement ("connections");
		Haruhi::ConnSet cset (true);
		cset.insert_unit (_mikuru);
		cset.save();
		cset.save_state (connections_element);
		element.appendChild (connections_element);
	}
}


void
Patch::load_state (QDomElement const& element)
{
	Parameters parameters;
	QDomElement general_element;
	QDomElement common_filters_element;
	DomElements part_elements;
	DomElements envelope_elements;
	QDomElement connections_element;

	if (element.attribute ("version") != Patch::VERSION_D_2010_02_20)
	{
		QMessageBox::warning (_mikuru, "Error loading patch", "Unknown patch version \"" + element.attribute ("version") + "\", not loading.");
		return;
	}

	// Remove all current parts and envelopes:
	_mikuru->del_all_parts();
	_mikuru->general()->envelopes()->destroy_all_envelopes();

	// Setup parameters map and part_elements list:
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "part")
				part_elements.push_back (e);
			else if (e.tagName() == "envelope")
				envelope_elements.push_back (e);
			else if (e.tagName() == "common-filters")
				common_filters_element = e;
			else if (e.tagName() == "general")
				general_element = e;
			else if (e.tagName() == "connections")
				connections_element = e;
		}
	}

	// General:
	{
		General* general = _mikuru->general();
		Params::General* params = general->params();

		create_parameters (general_element, parameters);

		load_parameter (parameters, "polyphony", params->polyphony);
		load_parameter (parameters, "enable-audio-input", params->enable_audio_input);
		load_parameter (parameters, "volume", general->_knob_volume);
		load_parameter (parameters, "detune", general->_knob_detune);
		load_parameter (parameters, "panorama", general->_knob_panorama);
		load_parameter (parameters, "stereo-width", general->_knob_stereo_width);
		load_parameter (parameters, "input-volume", general->_knob_input_volume);

		general->load_params();
	}

	// Common filters:
	if (!common_filters_element.isNull())
	{
		CommonFilters* common_filters = _mikuru->common_filters();
		Params::CommonFilters* params = common_filters->params();

		create_parameters (common_filters_element, parameters);

		load_parameter (parameters, "filter-configuration", params->filter_configuration);
		load_parameter (parameters, "route-audio-input", params->route_audio_input);

		for (QDomNode n = common_filters_element.firstChild(); !n.isNull(); n = n.nextSibling())
		{
			QDomElement e = n.toElement();
			if (!e.isNull())
			{
				if (e.tagName() == "filter1" || e.tagName() == "filter2")
				{
					Filter* filter = e.tagName() == "filter1" ? common_filters->filter1() : common_filters->filter2();
					Params::Filter* params = filter->params();

					create_parameters (e, parameters);

					load_parameter (parameters, "enabled", params->enabled);
					load_parameter (parameters, "type", params->type);
					load_parameter (parameters, "passes", params->passes);
					load_parameter (parameters, "limiter-enabled", params->limiter_enabled);
					load_parameter (parameters, "frequency", filter->_knob_frequency);
					load_parameter (parameters, "resonance", filter->_knob_resonance);
					load_parameter (parameters, "gain", filter->_knob_gain);
					load_parameter (parameters, "attenuation", filter->_knob_attenuation);

					filter->load_params();
				}
			}
		}

		common_filters->load_params();
	}

	// For each <part> element:
	for (DomElements::iterator p = part_elements.begin(); p != part_elements.end(); ++p)
	{
		Part* part = _mikuru->add_part();

		// Load general params:
		{
			Params::Part* params = part->params();
			create_parameters (*p, parameters);
			load_parameter (parameters, "enabled", params->enabled);
			part->load_params();
		}

		for (QDomNode n = p->firstChild(); !n.isNull(); n = n.nextSibling())
		{
			QDomElement e = n.toElement();
			if (!e.isNull())
			{
				if (e.tagName() == "oscillator")
				{
					Oscillator* oscillator = part->oscillator();
					Params::Waveform* waveform_params = oscillator->waveform_params();
					Params::Oscillator* oscillator_params = oscillator->oscillator_params();
					Params::Voice* voice_params = oscillator->voice_params();

					create_parameters (e, parameters);

					// Knobs&other:
					load_parameter (parameters, "wave-type", waveform_params->wave_type);
					load_parameter (parameters, "wave-shape", waveform_params->wave_shape);
					load_parameter (parameters, "modulator-type", waveform_params->modulator_type);
					load_parameter (parameters, "modulator-wave-type", waveform_params->modulator_wave_type);
					load_parameter (parameters, "modulator-amplitude", waveform_params->modulator_amplitude);
					load_parameter (parameters, "modulator-index", waveform_params->modulator_index);
					load_parameter (parameters, "modulator-shape", waveform_params->modulator_shape);
					for (Oscillator::Sliders::size_type i = 0; i < Params::Waveform::HarmonicsNumber; ++i)
						load_parameter (parameters, QString ("harmonic-%1").arg (i), waveform_params->harmonics[i]);
					for (Oscillator::Sliders::size_type i = 0; i < Params::Waveform::HarmonicsNumber; ++i)
						load_parameter (parameters, QString ("phase-%1").arg (i), waveform_params->phases[i]);

					// Knobs:
					load_parameter (parameters, "volume", oscillator->_knob_volume);
					load_parameter (parameters, "portamento-time", oscillator->_knob_portamento_time);
					load_parameter (parameters, "phase", oscillator->_knob_phase);
					load_parameter (parameters, "noise-level", oscillator->_knob_noise_level);
					// Other:
					load_parameter (parameters, "wave-enabled", oscillator_params->wave_enabled);
					load_parameter (parameters, "noise-enabled", oscillator_params->noise_enabled);
					load_parameter (parameters, "frequency-modulation-range", oscillator_params->frequency_mod_range);
					load_parameter (parameters, "pitchbend-enabled", oscillator_params->pitchbend_enabled);
					load_parameter (parameters, "pitchbend-released", oscillator_params->pitchbend_released);
					load_parameter (parameters, "pitchbend-up-semitones", oscillator_params->pitchbend_up_semitones);
					load_parameter (parameters, "pitchbend-down-semitones", oscillator_params->pitchbend_down_semitones);
					load_parameter (parameters, "transposition-semitones", oscillator_params->transposition_semitones);
					load_parameter (parameters, "monophonic", oscillator_params->monophonic);
					load_parameter (parameters, "monophonic-retrigger", oscillator_params->monophonic_retrigger);
					load_parameter (parameters, "monophonic-key-priority", oscillator_params->monophonic_key_priority);
					load_parameter (parameters, "const-portamento-time", oscillator_params->const_portamento_time);

					// Knobs:
					load_parameter (parameters, "amplitude", voice_params->amplitude);
					load_parameter (parameters, "frequency", voice_params->frequency);
					load_parameter (parameters, "panorama", oscillator->_knob_panorama);
					load_parameter (parameters, "detune", oscillator->_knob_detune);
					load_parameter (parameters, "pitchbend", oscillator->_knob_pitchbend);
					load_parameter (parameters, "velocity-sens", oscillator->_knob_velocity_sens);
					load_parameter (parameters, "unison-index", oscillator->_knob_unison_index);
					load_parameter (parameters, "unison-spread", oscillator->_knob_unison_spread);
					load_parameter (parameters, "unison-init", oscillator->_knob_unison_init);
					load_parameter (parameters, "unison-noise", oscillator->_knob_unison_noise);

					oscillator->load_waveform_params();
					oscillator->load_oscillator_params();
					oscillator->load_voice_params();
				}
				else if (e.tagName() == "filters")
				{
					PartFilters* part_filters = part->filters();
					Params::PartFilters* params = part_filters->params();

					create_parameters (e, parameters);

					load_parameter (parameters, "filter-configuration", params->filter_configuration);

					for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
					{
						QDomElement e = n.toElement();
						if (!e.isNull())
						{
							if (e.tagName() == "filter1" || e.tagName() == "filter2")
							{
								Filter* filter = e.tagName() == "filter1" ? part_filters->filter1() : part_filters->filter2();
								Params::Filter* params = filter->params();

								create_parameters (e, parameters);

								// Knobs:
								load_parameter (parameters, "frequency", filter->_knob_frequency);
								load_parameter (parameters, "resonance", filter->_knob_resonance);
								load_parameter (parameters, "gain", filter->_knob_gain);
								load_parameter (parameters, "attenuation", filter->_knob_attenuation);
								// Other:
								load_parameter (parameters, "enabled", params->enabled);
								load_parameter (parameters, "type", params->type);
								load_parameter (parameters, "passes", params->passes);
								load_parameter (parameters, "limiter-enabled", params->limiter_enabled);

								filter->load_params();
							}
						}
					}

					part_filters->load_params();
				}
				else if (e.tagName() == "effect")
				{
					if (e.attribute ("type") == "waveshaper")
					{
						Waveshaper* waveshaper = part->effects()->add_waveshaper (e.attribute ("id").toInt());
						Params::Waveshaper* params = waveshaper->params();

						create_parameters (e, parameters);

						// Knobs:
						load_parameter (parameters, "gain", waveshaper->_knob_gain);
						load_parameter (parameters, "parameter", waveshaper->_knob_parameter);
						// Other:
						load_parameter (parameters, "enabled", params->enabled);
						load_parameter (parameters, "type", params->type);

						waveshaper->load_params();
					}
				}
			}
		}
	}

	// For each <envelope> element:
	for (DomElements::iterator en = envelope_elements.begin(); en != envelope_elements.end(); ++en)
	{
		if (en->attribute ("type") == "adsr")
		{
			ADSR* adsr = _mikuru->general()->envelopes()->add_adsr (en->attribute ("id").toInt());
			Params::ADSR* params = adsr->params();

			create_parameters (*en, parameters);

			// Knobs:
			load_parameter (parameters, "delay", adsr->_knob_delay);
			load_parameter (parameters, "attack", adsr->_knob_attack);
			load_parameter (parameters, "attack-hold", adsr->_knob_attack_hold);
			load_parameter (parameters, "decay", adsr->_knob_decay);
			load_parameter (parameters, "sustain", adsr->_knob_sustain);
			load_parameter (parameters, "sustain-hold", adsr->_knob_sustain_hold);
			load_parameter (parameters, "release", adsr->_knob_release);
			// Other:
			load_parameter (parameters, "enabled", params->enabled);
			load_parameter (parameters, "direct-adsr", params->direct_adsr);
			load_parameter (parameters, "forced-release", params->forced_release);
			load_parameter (parameters, "sustain-enabled", params->sustain_enabled);
			load_parameter (parameters, "function", params->function);
			load_parameter (parameters, "mode", params->mode);

			adsr->load_params();
		}
		else if (en->attribute ("type") == "eg")
		{
			EG* eg = _mikuru->general()->envelopes()->add_eg (en->attribute ("id").toInt());
			Params::EG* params = eg->params();

			create_parameters (*en, parameters);

			// Other:
			load_parameter (parameters, "enabled", params->enabled);
			load_parameter (parameters, "segments", params->segments);
			load_parameter (parameters, "sustain-point", params->sustain_point);
			// Load envelope points:
			DSP::Envelope::Points& points = eg->envelope_template().points();
			points.clear();
			for (QDomNode n = en->firstChild(); !n.isNull(); n = n.nextSibling())
			{
				QDomElement e = n.toElement();
				if (!e.isNull() && e.tagName() == "points")
				{
					int i = 0;
					for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
					{
						QDomElement e = n.toElement();
						if (!e.isNull() && e.tagName() == "point")
						{
							params->values[i] = e.attribute ("value").toInt();
							params->durations[i] = e.attribute ("duration").toInt();
							i += 1;
						}
					}
				}
			}

			eg->load_params();
		}
		else if (en->attribute ("type") == "lfo")
		{
			LFO* lfo = _mikuru->general()->envelopes()->add_lfo (en->attribute ("id").toInt());
			Params::LFO* params = lfo->params();

			create_parameters (*en, parameters);

			// Knobs:
			load_parameter (parameters, "delay", lfo->_knob_delay);
			load_parameter (parameters, "fade-in", lfo->_knob_fade_in);
			load_parameter (parameters, "frequency", lfo->_knob_frequency);
			load_parameter (parameters, "level", lfo->_knob_level);
			load_parameter (parameters, "depth", lfo->_knob_depth);
			load_parameter (parameters, "phase", lfo->_knob_phase);
			load_parameter (parameters, "wave-shape", lfo->_knob_wave_shape);
			load_parameter (parameters, "fade-out", lfo->_knob_fade_out);
			// Other:
			load_parameter (parameters, "enabled", params->enabled);
			load_parameter (parameters, "wave-type", params->wave_type);
			load_parameter (parameters, "wave-invert", params->wave_invert);
			load_parameter (parameters, "function", params->function);
			load_parameter (parameters, "mode", params->mode);
			load_parameter (parameters, "tempo-sync", params->tempo_sync);
			load_parameter (parameters, "tempo-numerator", params->tempo_numerator);
			load_parameter (parameters, "tempo-denominator", params->tempo_denominator);
			load_parameter (parameters, "random-start-phase", params->random_start_phase);
			load_parameter (parameters, "fade-out-enabled", params->fade_out_enabled);

			lfo->load_params();
		}
	}

	_mikuru->general()->envelopes()->show_first();

	// Connections:
	{
		Haruhi::ConnSet cset;
		cset.insert_unit (_mikuru);
		cset.load_state (connections_element);
		cset.load();
	}
}


void
Patch::create_parameters (QDomElement const& element, Parameters& parameters) const
{
	parameters.clear();
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
			parameters[e.tagName()] = e;
	}
}

} // namespace MikuruPrivate

