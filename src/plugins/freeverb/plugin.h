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

#ifndef HARUHI__PLUGINS__FREEVERB__PLUGIN_H__INCLUDED
#define HARUHI__PLUGINS__FREEVERB__PLUGIN_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/ramp_smoother.h>
#include <haruhi/session/unit_bay.h>
#include <haruhi/plugin/plugin.h>
#include <haruhi/plugin/has_presets.h>
#include <haruhi/graph/audio_port.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/lib/controller_param.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "reverb_model.h"


namespace Freeverb {

using Haruhi::Unique;
namespace DSP = Haruhi::DSP;


class Plugin:
	public Haruhi::Plugin,
	public Haruhi::UnitBayAware,
	public Haruhi::HasPresets,
	public SaveableState
{
	Q_OBJECT

  public:
	Plugin (std::string const& urn, std::string const& title, int id, QWidget* parent);

	/*
	 * Plugin implementation.
	 */

	void
	registered() override;

	void
	process() override;

	void
	panic() override;

	void
	graph_updated() override;

	void
	set_unit_bay (Haruhi::UnitBay*) override;

	/*
	 * SaveableState implementation
	 */

	void
	save_state (QDomElement&) const override;

	void
	load_state (QDomElement const&) override;

	/*
	 * HasPresets implementation.
	 */

	void
	save_preset (QDomElement& element) const override;

	void
	load_preset (QDomElement const& element) override;

  private:
	std::array<Haruhi::Knob*, 4>
	knobs() const;

	/**
	 * Prepare dry-wet mix buffer that will contain smoothed vector
	 * of values used for mixing dry and wet sounds.
	 *
	 * \param	param
	 * 			Value of dry-wet param to use.
	 */
	void
	prepare_drywet_buffer (Haruhi::AudioBuffer*, float param);

  private:
	ReverbModel						_reverb_model;

	// In this order:
	Unique<Haruhi::EventPort>		_port_drywet;
	Unique<Haruhi::EventPort>		_port_room_size;
	Unique<Haruhi::EventPort>		_port_width;
	Unique<Haruhi::EventPort>		_port_damping;
	Unique<Haruhi::AudioPort>		_in[2];
	Unique<Haruhi::AudioPort>		_out[2];

	Unique<Haruhi::v06::ControllerParam>	_param_drywet;
	Unique<Haruhi::v06::ControllerParam>	_param_room_size;
	Unique<Haruhi::v06::ControllerParam>	_param_width;
	Unique<Haruhi::v06::ControllerParam>	_param_damping;

	Unique<Haruhi::Knob>			_knob_drywet;
	Unique<Haruhi::Knob>			_knob_room_size;
	Unique<Haruhi::Knob>			_knob_width;
	Unique<Haruhi::Knob>			_knob_damping;

	DSP::RampSmoother				_param_drywet_smoother;
	Haruhi::AudioBuffer				_drywet_mix_buffer;
	std::map<std::string, Haruhi::v06::ControllerParam*>
									_param_names;
};


inline void
Plugin::save_preset (QDomElement& element) const
{
	save_state (element);
}


inline void
Plugin::load_preset (QDomElement const& element)
{
	load_state (element);
}

} // namespace Freeverb

#endif

