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

// Qt:
#include <QLayout>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>
#include <haruhi/utility/qdom.h>

// Local:
#include "plugin.h"


namespace Freeverb {

Plugin::Plugin (std::string const& urn, std::string const& title, int id, QWidget* parent):
	Haruhi::Plugin (urn, title, id, parent),
	_reverb_model (48000_Hz)
{
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	_port_drywet		= std::make_unique<Haruhi::EventPort> (this, "Dry/wet", Haruhi::Port::Input);
	_port_room_size		= std::make_unique<Haruhi::EventPort> (this, "Room size", Haruhi::Port::Input);
	_port_width			= std::make_unique<Haruhi::EventPort> (this, "Width", Haruhi::Port::Input);
	_port_damping		= std::make_unique<Haruhi::EventPort> (this, "Damping", Haruhi::Port::Input);

	_in[0]				= std::make_unique<Haruhi::AudioPort> (this, "In 1", Haruhi::Port::Input);
	_in[1]				= std::make_unique<Haruhi::AudioPort> (this, "In 2", Haruhi::Port::Input);

	_out[0]				= std::make_unique<Haruhi::AudioPort> (this, "Out 1", Haruhi::Port::Output);
	_out[1]				= std::make_unique<Haruhi::AudioPort> (this, "Out 2", Haruhi::Port::Output);

	_param_drywet		= std::make_unique<Haruhi::ControllerParam> (Range<int> { 0, 1000 }, 200, 0, 1000, "dry-wet", Range<float> { 0.0, 1.0 }, 2, 1);
	_param_room_size	= std::make_unique<Haruhi::ControllerParam> (Range<int> { 0, 1000 }, 0, 0, 1000, "room-size", Range<float> { 0.0, 1.0 }, 2, 1);
	_param_width		= std::make_unique<Haruhi::ControllerParam> (Range<int> { 0, 1000 }, 0, 0, 1000, "width", Range<float> { 0.0, 1.0 }, 2, 1);
	_param_damping		= std::make_unique<Haruhi::ControllerParam> (Range<int> { 0, 1000 }, 0, 0, 1000, "damping", Range<float> { 0.0, 1.0 }, 2, 1);

	_knob_drywet		= std::make_unique<Haruhi::Knob> (this, _port_drywet.get(), _param_drywet.get(), "Dry/wet");
	_knob_room_size		= std::make_unique<Haruhi::Knob> (this, _port_room_size.get(), _param_room_size.get(), "Room size");
	_knob_width			= std::make_unique<Haruhi::Knob> (this, _port_width.get(), _param_width.get(), "Width");
	_knob_damping		= std::make_unique<Haruhi::Knob> (this, _port_damping.get(), _param_damping.get(), "Damping");

	for (auto knob: knobs())
		knob->set_unit_bay (unit_bay());

	_param_names = {
		{ "dry-wet",	_param_drywet.get() },
		{ "room-size",	_param_room_size.get() },
		{ "width",		_param_width.get() },
		{ "damping",	_param_damping.get() },
	};

	// TODO logo from SVG

	auto layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addWidget (_knob_drywet.get());
	layout->addWidget (_knob_room_size.get());
	layout->addWidget (_knob_width.get());
	layout->addWidget (_knob_damping.get());
}


void
Plugin::registered()
{
	graph_updated(); // Initially resize buffers.
	enable();
}


void
Plugin::process()
{
	sync_inputs();
	clear_outputs();

	for (auto knob: knobs())
		knob->controller_proxy()->process_events();

	_reverb_model.set_room_size (_param_room_size->to_f());
	_reverb_model.set_width (_param_width->to_f());
	_reverb_model.set_damping (_param_damping->to_f());
	_reverb_model.set_mode (ReverbModel::Mode::Normal); // TODO configurable

	auto buf_i_0 = _in[0]->buffer();
	auto buf_i_1 = _in[1]->buffer();
	auto buf_o_0 = _out[0]->buffer();
	auto buf_o_1 = _out[1]->buffer();

	_reverb_model.process (buf_i_0->begin(), buf_i_1->begin(), buf_o_0->begin(), buf_o_1->begin(), buf_i_0->size());

	// Attenuation for wet output:
	prepare_drywet_buffer (&_drywet_mix_buffer, _param_drywet->to_f());

	// Wet:
	buf_o_0->attenuate (&_drywet_mix_buffer);
	buf_o_1->attenuate (&_drywet_mix_buffer);

	// Attenuation for dry output:
	for (auto& s: _drywet_mix_buffer)
		s = 1.0 - s;

	// Dry:
	buf_o_0->mixin (buf_i_0, &_drywet_mix_buffer);
	buf_o_1->mixin (buf_i_1, &_drywet_mix_buffer);
}


void
Plugin::panic()
{
	_reverb_model.clear();
}


void
Plugin::graph_updated()
{
	Unit::graph_updated();
	_reverb_model.set_sample_rate (graph()->sample_rate());
	_drywet_mix_buffer.resize (graph()->buffer_size());
	_param_drywet_smoother.set_samples (5_ms * graph()->sample_rate());
}


void
Plugin::set_unit_bay (Haruhi::UnitBay* unit_bay)
{
	UnitBayAware::set_unit_bay (unit_bay);

	for (auto knob: knobs())
		knob->set_unit_bay (this->unit_bay());
}


void
Plugin::save_state (QDomElement& element) const
{
	QDomElement state = element.ownerDocument().createElement ("state");

	for (auto const& name_and_param: _param_names)
	{
		QDomElement e = state.ownerDocument().createElement (QString::fromStdString (name_and_param.first));
		name_and_param.second->save_state (e);
		state.appendChild (e);
	}

	element.appendChild (state);
}


void
Plugin::load_state (QDomElement const& element)
{
	disable();
	for (QDomElement& e: element)
	{
		if (e.tagName() == "state")
		{
			for (auto ep: e)
			{
				auto it = _param_names.find (ep.tagName().toStdString());
				if (it != _param_names.end())
					it->second->load_state (ep);
			}
			break;
		}
	}
	enable();
}


std::array<Haruhi::Knob*, 4>
Plugin::knobs() const
{
	return { _knob_drywet.get(), _knob_room_size.get(), _knob_width.get(), _knob_damping.get() };
}


void
Plugin::prepare_drywet_buffer (Haruhi::AudioBuffer* buffer, float param)
{
	_param_drywet_smoother.fill (buffer->begin(), buffer->end(), param);
	SIMD::power_buffer_to_scalar (buffer->begin(), buffer->size(), M_E);
}

} // namespace Freeverb

