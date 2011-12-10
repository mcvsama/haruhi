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
#include <haruhi/widgets/knob.h>

// Local:
#include "part_widget.h"
#include "part_manager_widget.h"
#include "part.h"


namespace Yuki {

PartWidget::PartWidget (PartManagerWidget* part_manager_widget, Part* part):
	QWidget (part_manager_widget),
	_part_manager_widget (part_manager_widget),
	_part (part)
{
	Params::Part* pp = _part->part_params();
	Params::Voice* vp = _part->voice_params();
	Part::PartPorts* ports = _part->ports();

	_knob_wave_shape			= new Haruhi::Knob (this, ports->wave_shape, &pp->wave_shape, "Shape");
	_knob_modulator_amplitude	= new Haruhi::Knob (this, ports->modulator_amplitude, &pp->modulator_amplitude, "Mod.amp.");
	_knob_modulator_index		= new Haruhi::Knob (this, ports->modulator_index, &pp->modulator_index, "Mod.index");
	_knob_modulator_shape		= new Haruhi::Knob (this, ports->modulator_shape, &pp->modulator_shape, "Mod.shape");
	_knob_volume				= new Haruhi::Knob (this, ports->volume, &pp->volume, "Volume dB");
	_knob_panorama				= new Haruhi::Knob (this, ports->panorama, &vp->panorama, "Panorama");
	_knob_detune				= new Haruhi::Knob (this, ports->detune, &vp->detune, "Detune");
	_knob_pitchbend				= new Haruhi::Knob (this, ports->pitchbend, &vp->pitchbend, "Pitch");
	_knob_unison_index			= new Haruhi::Knob (this, ports->unison_index, &vp->unison_index, "Unison");
	_knob_unison_spread			= new Haruhi::Knob (this, ports->unison_spread, &vp->unison_spread, "U.spread");
	_knob_unison_init			= new Haruhi::Knob (this, ports->unison_init, &vp->unison_init, "U.init.φ");
	_knob_unison_noise			= new Haruhi::Knob (this, ports->unison_noise, &vp->unison_noise, "U.noise");
	_knob_velocity_sens			= new Haruhi::Knob (this, ports->velocity_sens, &vp->velocity_sens, "Vel.sens.");
	_knob_portamento_time		= new Haruhi::Knob (this, ports->portamento_time, &pp->portamento_time, "Glide");
	_knob_phase					= new Haruhi::Knob (this, ports->phase, &pp->phase, "Phase");
	_knob_noise_level			= new Haruhi::Knob (this, ports->noise_level, &pp->noise_level, "Noise lvl");
}


PartWidget::~PartWidget()
{
	delete _knob_wave_shape;
	delete _knob_modulator_amplitude;
	delete _knob_modulator_index;
	delete _knob_modulator_shape;
	delete _knob_volume;
	delete _knob_panorama;
	delete _knob_detune;
	delete _knob_pitchbend;
	delete _knob_velocity_sens;
	delete _knob_unison_index;
	delete _knob_unison_spread;
	delete _knob_unison_init;
	delete _knob_unison_noise;
	delete _knob_portamento_time;
	delete _knob_phase;
	delete _knob_noise_level;
}

} // namespace Yuki

