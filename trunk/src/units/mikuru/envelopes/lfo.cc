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
#include <algorithm>
#include <stdint.h>
#include <cmath>
#include <set>

// Qt:
#include <QtGui/QToolTip>
#include <QtGui/QPushButton>
#include <QtGui/QGroupBox>
#include <QtGui/QGridLayout>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/functions.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/widgets/wave_plot.h>

// Local:
#include "../mikuru.h"
#include "../envelopes.h"
#include "../voice.h"
#include "../voice_manager.h"
#include "../part.h"
#include "lfo.h"


namespace MikuruPrivate {

LFO::Osc::Osc (float start_phase):
	_wave (0),
	_wave_is_random (false),
	_phase (start_phase),
	_current_delay_sample (0),
	_current_fade_in_sample (0),
	_current_fade_out_sample (0)
{ }


void
LFO::Osc::reset (float start_phase)
{
	_phase = start_phase;
	_current_delay_sample = 0;
	_current_fade_in_sample = 0;
	_current_fade_out_sample = 0;
}


Haruhi::Sample
LFO::Osc::advance (unsigned int samples)
{
	if (!_wave)
		return 0.0f;

	if (_current_delay_sample < _delay_samples)
	{
		_current_delay_sample += samples;
		return (_level + 1.0f) / 2.0f;
	}
	else
	{
		float dep = _depth;
		float prev_phase = _phase;

		_phase = mod1 (_phase + _frequency * samples);

		if (_wave_is_random && prev_phase > _phase)
			static_cast<RandomWave*> (_wave)->next_step();

		// Fade in:
		if (_current_fade_in_sample < _fade_in_samples)
		{
			_current_fade_in_sample += samples;
			dep *= std::pow (1.0f * std::min (_current_fade_in_sample, _fade_in_samples) / _fade_in_samples, 2.0f);
		}
		// Fade out:
		else if (_fade_out_enabled)
		{
			_current_fade_out_sample += samples;
			dep *= std::pow (1.0f - 1.0f * std::min (_current_fade_out_sample, _fade_out_samples) / _fade_out_samples, 2.0f);
		}

		// Osc:
		float lev = _level * (1.0f - dep);
		return ((lev + _inverter * (*_wave) (_phase, 0.0f) * dep) + 1.0f) / 2.0f;
	}
}


LFO::RandomWave::RandomWave (Type type):
	ParametricWave (false),
	_type (type)
{
	_prev_value = noise_sample();
	_curr_value = noise_sample();
}


void
LFO::RandomWave::next_step()
{
	_prev_value = _curr_value;
	_curr_value = noise_sample();
}


Haruhi::Sample
LFO::RandomWave::operator() (Haruhi::Sample register phase, Haruhi::Sample) const
{
	switch (_type)
	{
		case Square:
			return _curr_value;
		case Triangle:
			return phase * (_curr_value - _prev_value) + _prev_value;
	}
}


LFO::LFO (int id, Mikuru* mikuru, QWidget* parent):
	Envelope (parent),
	_mikuru (mikuru),
	_loading_params (false),
	_pressed_keys (0)
{
	_id = (id == 0) ? _mikuru->allocate_id ("lfos") : _mikuru->reserve_id ("lfos", id);

	// Create Waves:
	_waves[Params::LFO::Sine] = new DSP::ParametricWaves::Sine();
	_waves[Params::LFO::Triangle] = new DSP::ParametricWaves::Triangle();
	_waves[Params::LFO::Square] = new DSP::ParametricWaves::Square();
	_waves[Params::LFO::Sawtooth] = new DSP::ParametricWaves::Sawtooth();
	_waves[Params::LFO::Pulse] = new DSP::ParametricWaves::Pulse();
	_waves[Params::LFO::RandomSquare] = new RandomWave (RandomWave::Square);
	_waves[Params::LFO::RandomTriangle] = new RandomWave (RandomWave::Triangle);

	QWidget* knobs_panel = new QWidget (this);
	knobs_panel->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Expanding);

	create_ports();
	create_knobs (knobs_panel);
	create_widgets (knobs_panel);
	update_params();
	update_plot();
	update_widgets();
	reset_common_osc();
}


LFO::~LFO()
{
	delete _knob_delay;
	delete _knob_fade_in;
	delete _knob_frequency;
	delete _knob_level;
	delete _knob_depth;
	delete _knob_phase;
	delete _knob_wave_shape;
	delete _knob_fade_out;

	_plot->assign_wave (0);
	_mikuru->free_id ("lfos", _id);

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	delete _port_delay;
	delete _port_fade_in;
	delete _port_frequency;
	delete _port_level;
	delete _port_depth;
	delete _port_phase;
	delete _port_output;
	delete _port_wave_shape;
	delete _port_fade_out;
	// Delete remaining Oscs:
	for (Oscs::iterator x = _oscs.begin(); x != _oscs.end(); ++x)
		delete x->second;
	if (_mikuru->graph())
		_mikuru->graph()->unlock();

	// Delete waves:
	for (Waves::iterator w = _waves.begin(); w != _waves.end(); ++w)
		delete w->second;
}


void
LFO::create_ports()
{
	if (_mikuru->graph())
		_mikuru->graph()->lock();
	_port_group = new Haruhi::PortGroup (_mikuru->graph(), QString ("LFO %1").arg (this->id()).toStdString());
	// Inputs:
	_port_delay = new Haruhi::EventPort (_mikuru, "Delay", Haruhi::Port::Input, _port_group);
	_port_fade_in = new Haruhi::EventPort (_mikuru, "Fade in", Haruhi::Port::Input, _port_group);
	_port_frequency = new Haruhi::EventPort (_mikuru, "Frequency", Haruhi::Port::Input, _port_group);
	_port_level = new Haruhi::EventPort (_mikuru, "Level", Haruhi::Port::Input, _port_group);
	_port_depth = new Haruhi::EventPort (_mikuru, "Depth", Haruhi::Port::Input, _port_group);
	_port_phase = new Haruhi::EventPort (_mikuru, "Start phase", Haruhi::Port::Input, _port_group);
	_port_wave_shape = new Haruhi::EventPort (_mikuru, "Wave shape", Haruhi::Port::Input, _port_group);
	_port_fade_out = new Haruhi::EventPort (_mikuru, "Fade out", Haruhi::Port::Input, _port_group);
	// Outputs:
	_port_output = new Haruhi::EventPort (_mikuru, QString ("LFO %1").arg (this->id()).toStdString(), Haruhi::Port::Output, 0, Haruhi::Port::Polyphonic);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();
}


void
LFO::create_knobs (QWidget* parent)
{
	_knob_delay = new Haruhi::Knob (parent, _port_delay, &_params.delay, "Delay", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::Delay, 1000), 2);
	_knob_fade_in = new Haruhi::Knob (parent, _port_fade_in, &_params.fade_in, "Fade in", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::FadeIn, 1000), 2);
	_knob_frequency = new Haruhi::Knob (parent, _port_frequency, &_params.frequency, "Frequency", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::Frequency, 3000), 3);
	_knob_level = new Haruhi::Knob (parent, _port_level, &_params.level, "Level", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::Level, 100), 2);
	_knob_depth = new Haruhi::Knob (parent, _port_depth, &_params.depth, "Depth", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::Depth, 100), 2);
	_knob_phase = new Haruhi::Knob (parent, _port_phase, &_params.phase, "Phase", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::Phase, 1000), 3);
	_knob_wave_shape = new Haruhi::Knob (parent, _port_wave_shape, &_params.wave_shape, "W.shape", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::WaveShape, 1000), 3);
	_knob_fade_out = new Haruhi::Knob (parent, _port_fade_out, &_params.fade_out, "Fade out", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::FadeOut, 100), 2);

	// Proxy configs:
	Haruhi::Knob* curved_knobs[] = {
		_knob_delay, _knob_fade_in, _knob_fade_out
	};
	for (Haruhi::Knob** k = curved_knobs; k != curved_knobs + sizeof (curved_knobs) / sizeof (*curved_knobs); ++k)
	{
		(*k)->controller_proxy().config().curve = 1.0;
		(*k)->controller_proxy().apply_config();
	}

	_knob_frequency->controller_proxy().config().curve = 0.5;
	_knob_frequency->controller_proxy().apply_config();

	// Set unit bay:
	Haruhi::Knob* all_knobs[] = {
		_knob_delay, _knob_fade_in, _knob_frequency, _knob_level,
		_knob_depth, _knob_phase, _knob_wave_shape, _knob_fade_out
	};
	for (Haruhi::Knob** k = all_knobs; k != all_knobs + sizeof (all_knobs) / sizeof (*all_knobs); ++k)
		(*k)->set_unit_bay (_mikuru->unit_bay());

	QObject::connect (_knob_fade_out, SIGNAL (changed (int)), this, SLOT (update_plot()));
	QObject::connect (_knob_wave_shape, SIGNAL (changed (int)), this, SLOT (update_plot()));
	QObject::connect (_knob_phase, SIGNAL (changed (int)), this, SLOT (update_plot()));
}


void
LFO::create_widgets (QWidget* knobs_panel)
{
	Params::LFO p = _params;

	QToolTip::add (_knob_wave_shape, "Wave shape");
	QToolTip::add (_knob_phase, "Start phase");

	QFrame* plot_frame = new QFrame (knobs_panel);
	plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_plot = new Haruhi::WavePlot (plot_frame);
	QVBoxLayout* plot_frame_layout = new QVBoxLayout (plot_frame, 0, Config::Spacing);
	plot_frame_layout->addWidget (_plot);

	QGroupBox* grid1 = new QGroupBox (this);
	QGridLayout* grid1_layout = new QGridLayout (grid1);
	grid1->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	grid1_layout->setMargin (3 * Config::Margin);

	_enabled = new QCheckBox ("Enabled", grid1);
	_enabled->setChecked (p.enabled);
	grid1_layout->addWidget (_enabled, 0, 0);
	QObject::connect (_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	_wave_invert = new QCheckBox ("Invert wave", grid1);
	_wave_invert->setChecked (p.wave_invert);
	grid1_layout->addWidget (_wave_invert, 0, 1);
	QObject::connect (_wave_invert, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QObject::connect (_wave_invert, SIGNAL (toggled (bool)), this, SLOT (update_plot()));

	_random_start_phase = new QCheckBox ("Random phase", grid1);
	_random_start_phase->setChecked (p.random_start_phase);
	grid1_layout->addWidget (_random_start_phase, 1, 0);
	QObject::connect (_random_start_phase, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QObject::connect (_random_start_phase, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	_fade_out_enabled = new QCheckBox ("Fade out", grid1);
	_fade_out_enabled->setChecked (p.fade_out_enabled);
	grid1_layout->addWidget (_fade_out_enabled, 1, 1);
	QObject::connect (_fade_out_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QObject::connect (_fade_out_enabled, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	_tempo_sync = new QCheckBox ("Sync with tempo", grid1);
	_tempo_sync->setChecked (p.tempo_sync);
	grid1_layout->addWidget (_tempo_sync, 2, 0);
	QObject::connect (_tempo_sync, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QObject::connect (_tempo_sync, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	QWidget* tempo_grid = new QWidget (grid1);
	QHBoxLayout* tempo_layout = new QHBoxLayout (tempo_grid, 0, Config::Spacing);
	tempo_layout->setAutoAdd (true);
	grid1_layout->addWidget (tempo_grid, 2, 1);

	_tempo_numerator = new QSpinBox (1, 64, 1, tempo_grid);
	_tempo_numerator->setValue (p.tempo_numerator);
	QToolTip::add (_tempo_numerator, "Tempo divider numerator");
	QObject::connect (_tempo_numerator, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	_tempo_denominator = new QSpinBox (1, 64, 1, tempo_grid);
	_tempo_denominator->setValue (p.tempo_denominator);
	QToolTip::add (_tempo_denominator, "Tempo divider denonimator");
	QObject::connect (_tempo_denominator, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	_wave_type = new QComboBox (grid1);
	_wave_type->insertItem (Resources::Icons16::wave_sine(), "Sine", Params::LFO::Sine);
	_wave_type->insertItem (Resources::Icons16::wave_triangle(), "Triangle", Params::LFO::Triangle);
	_wave_type->insertItem (Resources::Icons16::wave_square(), "Square", Params::LFO::Square);
	_wave_type->insertItem (Resources::Icons16::wave_sawtooth(), "Sawtooth", Params::LFO::Sawtooth);
	_wave_type->insertItem (Resources::Icons16::wave_pulse(), "Pulse", Params::LFO::Pulse);
	_wave_type->insertItem (Resources::Icons16::wave_random_square(), "Random square", Params::LFO::RandomSquare);
	_wave_type->insertItem (Resources::Icons16::wave_random_triangle(), "Random triangle", Params::LFO::RandomTriangle);
	_wave_type->setCurrentItem (p.wave_type);
	grid1_layout->addWidget (_wave_type, 3, 0, 1, 2);
	QObject::connect (_wave_type, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_wave_type, SIGNAL (activated (int)), this, SLOT (update_plot()));
	QObject::connect (_wave_type, SIGNAL (activated (int)), this, SLOT (update_widgets()));

	_function = new QComboBox (grid1);
	_function->insertItem ("Log. E", Params::LFO::LogarithmicE);
	_function->insertItem ("Log. 2", Params::LFO::Logarithmic2);
	_function->insertItem ("Linear", Params::LFO::Linear);
	_function->insertItem ("Exp. 2", Params::LFO::Expotential2);
	_function->insertItem ("Exp. E", Params::LFO::ExpotentialE);
	_function->setCurrentItem (p.function);
	grid1_layout->addWidget (_function, 4, 0);
	QObject::connect (_function, SIGNAL (activated (int)), this, SLOT (update_params()));

	_mode = new QComboBox (grid1);
	_mode->insertItem ("Polyphonic", Params::LFO::Polyphonic);
	_mode->insertItem ("Common Keysync", Params::LFO::CommonKeySync);
	_mode->insertItem ("Common Continuous", Params::LFO::CommonContinuous);
	_mode->setCurrentItem (p.mode);
	grid1_layout->addWidget (_mode, 4, 1);
	QObject::connect (_mode, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_mode, SIGNAL (activated (int)), this, SLOT (update_widgets()));

	// Layout:

	QVBoxLayout* v1 = new QVBoxLayout (knobs_panel, 0, Config::Spacing);
	QHBoxLayout* h1 = new QHBoxLayout (v1, Config::Spacing);
	h1->addWidget (_knob_fade_in);
	h1->addWidget (_knob_frequency);
	h1->addWidget (_knob_level);
	h1->addWidget (_knob_depth);
	h1->addWidget (_knob_fade_out);
	QHBoxLayout* h2 = new QHBoxLayout (v1, Config::Spacing);
	h2->addWidget (_knob_delay);
	h2->addWidget (plot_frame);
	h2->addWidget (_knob_wave_shape);
	h2->addWidget (_knob_phase);
	v1->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	QHBoxLayout* h3 = new QHBoxLayout (this, Config::Margin, Config::Spacing);
	h3->addWidget (knobs_panel);
	QVBoxLayout* v2 = new QVBoxLayout (h3, Config::Spacing);
	v2->addWidget (grid1);
	v2->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}


void
LFO::voice_created (VoiceManager*, Voice* voice)
{
	int mode = _params.mode.get();
	unsigned int sample_rate = _mikuru->graph()->sample_rate();

	// If this will be the first voice in the set and mode is CommonKeySync, reset common osc:
	if (_pressed_keys == 0 && mode == Params::LFO::CommonKeySync)
	{
		reset_common_osc();
		set_common_osc();
	}

	// If mode is Polyphonic:
	if (mode == Params::LFO::Polyphonic)
	{
		Osc* osc = new Osc (get_phase());
		osc->set_delay (_params.delay.to_f() * sample_rate);
		osc->set_fade_in (_params.fade_in.to_f() * sample_rate);
		osc->set_fade_out (_params.fade_out.to_f() * sample_rate);
		osc->set_fade_out_enabled (_params.fade_out_enabled.get());
		// Add osc to set:
		_oscs[voice] = osc;
	}

	_pressed_keys += 1;
}


void
LFO::voice_released (VoiceManager*, Voice* voice)
{
}


void
LFO::voice_dropped (VoiceManager*, Voice* voice)
{
	Oscs::iterator x = _oscs.find (voice);
	if (x != _oscs.end())
	{
		delete x->second;
		_oscs.erase (x);
	}

	_pressed_keys -= 1;
}


void
LFO::process()
{
	// Sync input ports by hand, since we're before Mikuru's autosync:
	_port_delay->sync();
	_port_fade_in->sync();
	_port_frequency->sync();
	_port_level->sync();
	_port_depth->sync();
	_port_phase->sync();
	_port_wave_shape->sync();
	_port_fade_out->sync();

	// Process ports events:
	_knob_delay->controller_proxy().process_events();
	_knob_fade_in->controller_proxy().process_events();
	_knob_frequency->controller_proxy().process_events();
	_knob_level->controller_proxy().process_events();
	_knob_depth->controller_proxy().process_events();
	_knob_phase->controller_proxy().process_events();
	_knob_wave_shape->controller_proxy().process_events();
	_knob_fade_out->controller_proxy().process_events();

	// Skip if disabled or not connected:
	if (!_params.enabled.get() || _port_output->forward_connections().empty())
		return;

	int wave_type = _params.wave_type.get();
	Haruhi::Timestamp t = _mikuru->graph()->timestamp();
	unsigned int sample_rate = _mikuru->graph()->sample_rate();
	unsigned int buffer_size = _mikuru->graph()->buffer_size();
	DSP::ParametricWave* wave = _waves[wave_type];
	update_wave_param();
	float frequency = _params.frequency.to_f() / sample_rate;
	float level = _params.level.to_f();
	float depth = _params.depth.to_f();
	bool invert = _params.wave_invert.get();

	// Assuming that output ports are cleared by Mikuru on beginnig of each
	// processing round.

	// Always process remaining polyphonic oscs:
	for (Oscs::iterator a = _oscs.begin(); a != _oscs.end(); ++a)
	{
		Voice* voice = a->first;
		Osc* osc = a->second;
		osc->set_wave (wave, wave_type == Params::LFO::RandomSquare || wave_type == Params::LFO::RandomTriangle);
		osc->set_frequency (frequency);
		osc->set_level (level);
		osc->set_depth (depth);
		osc->set_invert (invert);
		_port_output->event_buffer()->push (new Haruhi::VoiceControllerEvent (t, voice->voice_id(), apply_function (osc->advance (buffer_size))));
	}

	int mode = _params.mode.get();
	if ((mode == Params::LFO::CommonKeySync && _pressed_keys > 0) || mode == Params::LFO::CommonContinuous)
	{
		_common_osc.set_wave (wave, wave_type == Params::LFO::RandomSquare || wave_type == Params::LFO::RandomTriangle);
		_common_osc.set_frequency (frequency);
		_common_osc.set_level (level);
		_common_osc.set_depth (depth);
		_common_osc.set_invert (invert);
		_port_output->event_buffer()->push (new Haruhi::ControllerEvent (t, apply_function (_common_osc.advance (buffer_size))));
	}
}


void
LFO::resize_buffers (std::size_t)
{
	// Update LFO if sample rate changed:
	set_common_osc();
}


void
LFO::load_params()
{
	// Copy params:
	Params::LFO p (_params);
	_loading_params = true;

	_enabled->setChecked (p.enabled);
	_wave_type->setCurrentItem (p.wave_type);
	_wave_invert->setChecked (p.wave_invert);
	_function->setCurrentItem (p.function);
	_mode->setCurrentItem (p.mode);
	_tempo_sync->setChecked (p.tempo_sync);
	_tempo_numerator->setValue (p.tempo_numerator);
	_tempo_denominator->setValue (p.tempo_denominator);
	_random_start_phase->setChecked (p.random_start_phase);
	_fade_out_enabled->setChecked (p.fade_out_enabled);

	_loading_params = false;
	update_widgets();
}


void
LFO::load_params (Params::LFO& params)
{
	_params = params;
	load_params();
}


void
LFO::update_params()
{
	if (_loading_params)
		return;

	int prev_mode = _params.mode.get();

	_params.enabled.set (_enabled->isChecked());
	_params.wave_type.set (_wave_type->currentItem());
	_params.wave_invert.set (_wave_invert->isChecked());
	_params.function.set (_function->currentItem());
	_params.mode.set (_mode->currentItem());
	_params.tempo_sync.set (_tempo_sync->isChecked());
	_params.tempo_numerator.set (_tempo_numerator->value());
	_params.tempo_denominator.set (_tempo_denominator->value());
	_params.random_start_phase.set (_random_start_phase->isChecked());
	_params.fade_out_enabled.set (_fade_out_enabled->isChecked());

	// Knob params are updated automatically using #assign_parameter.

	if (prev_mode != _params.mode.get())
		reset_common_osc();
}


void
LFO::update_plot()
{
	bool random = _params.wave_type.get() == Params::LFO::RandomSquare || _params.wave_type.get() == Params::LFO::RandomTriangle;
	if (!random)
	{
		update_wave_param();
		_plot->assign_wave (_waves[_params.wave_type.get()], true, true, _params.wave_invert.get());
		_plot->set_phase_marker_position (_params.phase.to_f());
		_plot->plot_shape();
	}
}


void
LFO::update_widgets()
{
	bool random = _params.wave_type.get() == Params::LFO::RandomSquare || _params.wave_type.get() == Params::LFO::RandomTriangle;
	bool continuous = _params.mode.get() == Params::LFO::CommonContinuous;
	_knob_delay->setEnabled (!continuous);
	_knob_fade_in->setEnabled (!continuous);
	_knob_fade_out->setEnabled (!continuous && _params.fade_out_enabled);
	_knob_phase->setEnabled (!continuous && !_random_start_phase->isChecked());
	_knob_wave_shape->setEnabled (!random);
	_plot->setEnabled (!random);
	_plot->set_phase_marker_enabled (!continuous && !_random_start_phase->isChecked());
	_tempo_numerator->setEnabled (_tempo_sync->isChecked());
	_tempo_denominator->setEnabled (_tempo_sync->isChecked());
	update_plot();
}


void
LFO::update_wave_param()
{
	_waves[_params.wave_type.get()]->set_param (_params.wave_shape.to_f());
}


void
LFO::reset_common_osc()
{
	_common_osc.reset (get_phase());
	_common_osc.set_delay (0);
	_common_osc.set_fade_in (0);
	_common_osc.set_fade_out (0);
	_common_osc.set_fade_out_enabled (false);
}


void
LFO::set_common_osc()
{
	unsigned int sample_rate = _mikuru->graph()->sample_rate();
	// CommonKeySync supports all delay/fadeins:
	_common_osc.set_delay (_params.delay.to_f() * sample_rate);
	_common_osc.set_fade_in (_params.fade_in.to_f() * sample_rate);
	_common_osc.set_fade_out (_params.fade_out.to_f() * sample_rate);
	_common_osc.set_fade_out_enabled (_params.fade_out_enabled.get());
}


Haruhi::Sample
LFO::apply_function (Haruhi::Sample v) const
{
	switch (_params.function.get())
	{
		case Params::LFO::LogarithmicE: return std::pow (v, 1.0f/M_E);
		case Params::LFO::Logarithmic2: return std::pow (v, 0.5f);
		case Params::LFO::Linear:		return v;
		case Params::LFO::Expotential2: return std::pow (v, 2.0f);
		case Params::LFO::ExpotentialE: return std::pow (v, M_E);
	}
	return v;
}


float
LFO::get_phase() const
{
	return _params.random_start_phase.get()
		? _noise.get()
		: _params.phase.to_f();
}

} // namespace MikuruPrivate

