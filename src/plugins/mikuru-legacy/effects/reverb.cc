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
#include <QtGui/QPicture>
#include <QtGui/QLayout>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/simd_ops.h>

// Local:
#include "../mikuru.h"
#include "reverb.h"


namespace MikuruPrivate {

Reverb::CombFilter::CombFilter():
	_filter_store (0),
	_pos (0)
{
}


void
Reverb::CombFilter::set_buffer_size (size_t size)
{
	_buffer.resize (size);
}


void
Reverb::CombFilter::clear()
{
	_buffer.clear();
}


void
Reverb::CombFilter::set_damp (float value)
{
	_damp1 = value;
	_damp2 = 1.0f - value;
}


Reverb::AllpassFilter::AllpassFilter():
	_pos (0)
{
}


void
Reverb::AllpassFilter::set_buffer_size (size_t size)
{
	_buffer.resize (size);
}


void
Reverb::AllpassFilter::clear()
{
	_buffer.clear();
}


Reverb::Model::Model (unsigned int sample_rate):
	_gain (0.0f),
	_room_size (0.0f),
	_room_size1 (0.0f),
	_damp (0.0f),
	_damp1 (0.0f),
	_wet1 (0.0f),
	_wet2 (0.0f),
	_width (0.0f),
	_mode (NormalMode)
{
	scale_buffers (sample_rate);

	// Set default values:
	_allpassL[0].set_feedback (0.5f);
	_allpassR[0].set_feedback (0.5f);
	_allpassL[1].set_feedback (0.5f);
	_allpassR[1].set_feedback (0.5f);
	_allpassL[2].set_feedback (0.5f);
	_allpassR[2].set_feedback (0.5f);
	_allpassL[3].set_feedback (0.5f);
	_allpassR[3].set_feedback (0.5f);

	set_room_size (InitialRoom);
	set_damp (InitialDamp);
	set_width (InitialWidth);
	set_mode (NormalMode);

	// Initial forced update:
	update();

	clear();
}


void
Reverb::Model::clear()
{
	if (_mode == FreezeMode)
		return;

	for (int i = 0; i < NumCombs; ++i)
	{
		_combL[i].clear();
		_combR[i].clear();
	}

	for (int i = 0; i < NumAllpasses; ++i)
	{
		_allpassL[i].clear();
		_allpassR[i].clear();
	}
}


void
Reverb::Model::process (Sample* inputL, Sample* inputR, Sample* outputL, Sample* outputR, size_t samples)
{
	float outL, outR, input;

	while (samples-- > 0)
	{
		outL = outR = 0;
		input = (*inputL + *inputR) * _gain;

		// Accumulate comb filters in parallel:
		for (int i = 0; i < NumCombs; ++i)
		{
			outL += _combL[i].process (input);
			outR += _combR[i].process (input);
		}

		// Feed through allpasses in series:
		for(int i = 0; i < NumAllpasses; ++i)
		{
			outL = _allpassL[i].process (outL);
			outR = _allpassR[i].process (outR);
		}

		// Calculate output REPLACING anything already there:
		*outputL = outL * _wet1 + outR * _wet2;
		*outputR = outR * _wet1 + outL * _wet2;

		// Increment sample pointers:
		inputL += 1;
		inputR += 1;
		outputL += 1;
		outputR += 1;
	}
}


void
Reverb::Model::update()
{
	float wet = 3.0f;
	_wet1 = wet * (_width / 2.0f + 0.5f);
	_wet2 = wet * (1.0f - _width) / 2.0f;;

	if (_mode == FreezeMode)
	{
		_room_size1 = 1.0f;
		_damp1 = 0.0f;
		_gain = 0.0f;
	}
	else
	{
		_room_size1 = _room_size;
		_damp1 = _damp;
		_gain = FixedGain;
	}

	for (int i = 0; i < NumCombs; ++i)
	{
		_combL[i].set_feedback (_room_size1);
		_combR[i].set_feedback (_room_size1);
	}

	for(int i = 0; i < NumCombs; ++i)
	{
		_combL[i].set_damp (_damp1);
		_combR[i].set_damp (_damp1);
	}
}


void
Reverb::Model::set_room_size (float value)
{
	float s = FastPow::pow (value, 0.5f) * ScaleRoom + OffsetRoom;
	if (_room_size != s)
	{
		_room_size = s;
		update();
	}
}


void
Reverb::Model::set_damp (float value)
{
	if (_damp != value * ScaleDamp)
	{
		_damp = value * ScaleDamp;
		update();
	}
}


void
Reverb::Model::set_width (float value)
{
	if (_width != value)
	{
		_width = value;
		update();
	}
}


void
Reverb::Model::set_mode (Mode mode)
{
	if (_mode != mode)
	{
		_mode = mode;
		update();
	}
}


void
Reverb::Model::set_sample_rate (unsigned int sample_rate)
{
	scale_buffers (sample_rate);
	update();
}


void
Reverb::Model::scale_buffers (unsigned int sample_rate)
{
	// Original values are obtained for 44.1kHz, need to scale them.
	float scale_factor = sample_rate / 44100.0;

	_combL[0].set_buffer_size (scale_factor * CombTuningL1);
	_combR[0].set_buffer_size (scale_factor * CombTuningR1);
	_combL[1].set_buffer_size (scale_factor * CombTuningL2);
	_combR[1].set_buffer_size (scale_factor * CombTuningR2);
	_combL[2].set_buffer_size (scale_factor * CombTuningL3);
	_combR[2].set_buffer_size (scale_factor * CombTuningR3);
	_combL[3].set_buffer_size (scale_factor * CombTuningL4);
	_combR[3].set_buffer_size (scale_factor * CombTuningR4);
	_combL[4].set_buffer_size (scale_factor * CombTuningL5);
	_combR[4].set_buffer_size (scale_factor * CombTuningR5);
	_combL[5].set_buffer_size (scale_factor * CombTuningL6);
	_combR[5].set_buffer_size (scale_factor * CombTuningR6);
	_combL[6].set_buffer_size (scale_factor * CombTuningL7);
	_combR[6].set_buffer_size (scale_factor * CombTuningR7);
	_combL[7].set_buffer_size (scale_factor * CombTuningL8);
	_combR[7].set_buffer_size (scale_factor * CombTuningR8);

	_allpassL[0].set_buffer_size (scale_factor * AllpassTuningL1);
	_allpassR[0].set_buffer_size (scale_factor * AllpassTuningR1);
	_allpassL[1].set_buffer_size (scale_factor * AllpassTuningL2);
	_allpassR[1].set_buffer_size (scale_factor * AllpassTuningR2);
	_allpassL[2].set_buffer_size (scale_factor * AllpassTuningL3);
	_allpassR[2].set_buffer_size (scale_factor * AllpassTuningR3);
	_allpassL[3].set_buffer_size (scale_factor * AllpassTuningL4);
	_allpassR[3].set_buffer_size (scale_factor * AllpassTuningR4);
}


Reverb::Reverb (int id, Mikuru* mikuru, QWidget* parent):
	Effect (id, "reverbs", mikuru, "Reverb", new Params::Reverb(), parent),
	_mikuru (mikuru),
	_params (static_cast<Params::Reverb*> (Effect::params())),
	_loading_params (false),
	_reverb_model (mikuru->graph()->sample_rate())
{
	_freeze_mode = new QPushButton ("Freeze", this);
	_freeze_mode->setCheckable (true);
	_freeze_mode->setChecked (false);
	QObject::connect (_freeze_mode, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	_port_room_size = new Haruhi::EventPort (_mikuru, "Room size", Haruhi::Port::Input, port_group());
	_port_width = new Haruhi::EventPort (_mikuru, "Width", Haruhi::Port::Input, port_group());
	_port_damp = new Haruhi::EventPort (_mikuru, "Damp", Haruhi::Port::Input, port_group());
	if (_mikuru->graph())
		_mikuru->graph()->unlock();

	_knob_room_size = new Haruhi::Knob (this, _port_room_size, &_params->room_size, "Room size", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Reverb::RoomSize, 100), 2);
	_knob_width = new Haruhi::Knob (this, _port_width, &_params->width, "Width", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Reverb::Width, 100), 2);
	_knob_damp = new Haruhi::Knob (this, _port_damp, &_params->damp, "Damp", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Reverb::Damp, 100), 2);

	_knob_room_size->set_unit_bay (_mikuru->unit_bay());
	_knob_width->set_unit_bay (_mikuru->unit_bay());
	_knob_damp->set_unit_bay (_mikuru->unit_bay());

	QPixmap freeverb_logo (HARUHI_SHARED_DIRECTORY "/images/freeverb.png");

	QLabel* freeverb_logo_label = new QLabel (this);
	freeverb_logo_label->setPixmap (freeverb_logo);
	freeverb_logo_label->setMargin (5);

	QHBoxLayout* layout = new QHBoxLayout (parent_widget());
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (freeverb_logo_label);
	layout->setAlignment (freeverb_logo_label, Qt::AlignVCenter);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	layout->addWidget (_freeze_mode);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	layout->setAlignment (_freeze_mode, Qt::AlignVCenter);
	layout->addWidget (_knob_room_size);
	layout->addWidget (_knob_width);
	layout->addWidget (_knob_damp);
}


Reverb::~Reverb()
{
	delete _knob_room_size;
	delete _knob_width;
	delete _knob_damp;

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	delete _port_room_size;
	delete _port_width;
	delete _port_damp;
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
Reverb::graph_updated()
{
	_reverb_model.set_sample_rate (_mikuru->graph()->sample_rate());
}


void
Reverb::process_events()
{
	Effect::process_events();

	_knob_room_size->process_events();
	_knob_width->process_events();
	_knob_damp->process_events();
}


void
Reverb::process (Haruhi::AudioBuffer* in1, Haruhi::AudioBuffer* in2, Haruhi::AudioBuffer* out1, Haruhi::AudioBuffer* out2)
{
	_reverb_model.set_room_size (_params->room_size.to_f());
	_reverb_model.set_width (_params->width.to_f());
	_reverb_model.set_damp (_params->damp.to_f());
	_reverb_model.set_mode (_params->mode.get() ? Model::FreezeMode : Model::NormalMode);
	_reverb_model.process (in1->begin(), in2->begin(), out1->begin(), out2->begin(), in1->size());
}


void
Reverb::panic()
{
	_reverb_model.clear();
}


void
Reverb::load_params()
{
	// Copy params:
	Params::Reverb p (*_params);
	_loading_params = true;

	Effect::load_params();

	_freeze_mode->setChecked (p.mode);

	_loading_params = false;
}


void
Reverb::load_params (Params::Reverb& params)
{
	*_params = params;
	load_params();
}


void
Reverb::update_params()
{
	if (_loading_params)
		return;

	Effect::update_params();

	_params->mode.set (_freeze_mode->isChecked());

	// Knob params are updated automatically using #assign_parameter.

	_params->sanitize();
}

} // namespace MikuruPrivate

