/* vim:ts=4
 *
 * Copyleft 2008…2010  Michał Gawron
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
#include <Qt3Support/Q3GroupBox>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/controller_proxy.h>
#include <haruhi/dsp/functions.h>
#include <haruhi/widgets/wave_plot.h>

// Local:
#include "mikuru.h"
#include "envelopes.h"
#include "envelopes_lfo.h"
#include "voice.h"
#include "voice_manager.h"
#include "part.h"


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


Core::Sample
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


Core::Sample
LFO::RandomWave::operator() (Core::Sample register phase, Core::Sample) const
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
	create_proxies();
	create_knobs (knobs_panel);
	create_widgets (knobs_panel);
	update_plot();
	update_widgets();
	reset_common_osc();
}


LFO::~LFO()
{
	// Delete knobs before ControllerProxies:
	delete _control_delay;
	delete _control_fade_in;
	delete _control_frequency;
	delete _control_level;
	delete _control_depth;
	delete _control_phase;
	delete _control_wave_shape;
	delete _control_fade_out;

	delete _proxy_delay;
	delete _proxy_fade_in;
	delete _proxy_frequency;
	delete _proxy_level;
	delete _proxy_depth;
	delete _proxy_phase;
	delete _proxy_wave_shape;
	delete _proxy_fade_out;

	_plot->assign_wave (0);
	_mikuru->free_id ("lfos", _id);

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
	_mikuru->graph()->unlock();

	// Delete waves:
	for (Waves::iterator w = _waves.begin(); w != _waves.end(); ++w)
		delete w->second;
}


void
LFO::create_ports()
{
	_mikuru->graph()->lock();
	_port_group = new Core::PortGroup (_mikuru->graph(), QString ("LFO %1").arg (this->id()).toStdString());
	// Inputs:
	_port_delay = new Core::EventPort (_mikuru, "Delay", Core::Port::Input, _port_group);
	_port_fade_in = new Core::EventPort (_mikuru, "Fade in", Core::Port::Input, _port_group);
	_port_frequency = new Core::EventPort (_mikuru, "Frequency", Core::Port::Input, _port_group);
	_port_level = new Core::EventPort (_mikuru, "Level", Core::Port::Input, _port_group);
	_port_depth = new Core::EventPort (_mikuru, "Depth", Core::Port::Input, _port_group);
	_port_phase = new Core::EventPort (_mikuru, "Start phase", Core::Port::Input, _port_group);
	_port_wave_shape = new Core::EventPort (_mikuru, "Wave shape", Core::Port::Input, _port_group);
	_port_fade_out = new Core::EventPort (_mikuru, "Fade out", Core::Port::Input, _port_group);
	// Outputs:
	_port_output = new Core::EventPort (_mikuru, QString ("LFO %1").arg (this->id()).toStdString(), Core::Port::Output, 0, Core::Port::Polyphonic);
	_mikuru->graph()->unlock();
}


void
LFO::create_proxies()
{
	_proxy_delay = new Haruhi::ControllerProxy (_port_delay, &_params.delay);
	_proxy_delay->config()->curve = 1.0;
	_proxy_delay->apply_config();
	_proxy_fade_in = new Haruhi::ControllerProxy (_port_fade_in, &_params.fade_in);
	_proxy_fade_in->config()->curve = 1.0;
	_proxy_fade_in->apply_config();
	_proxy_frequency = new Haruhi::ControllerProxy (_port_frequency, &_params.frequency);
	_proxy_frequency->config()->curve = 0.5;
	_proxy_frequency->apply_config();
	_proxy_level = new Haruhi::ControllerProxy (_port_level, &_params.level);
	_proxy_depth = new Haruhi::ControllerProxy (_port_depth, &_params.depth);
	_proxy_phase = new Haruhi::ControllerProxy (_port_phase, &_params.phase);
	_proxy_wave_shape = new Haruhi::ControllerProxy (_port_wave_shape, &_params.wave_shape);
	_proxy_fade_out = new Haruhi::ControllerProxy (_port_fade_out, &_params.fade_out);
	_proxy_fade_out->config()->curve = 1.0;
	_proxy_fade_out->apply_config();
}


void
LFO::create_knobs (QWidget* parent)
{
	_control_delay = new Haruhi::Knob (parent, _proxy_delay, "Delay", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::Delay, 1000), 2);
	_control_delay->set_unit_bay (_mikuru->unit_bay());
	_control_fade_in = new Haruhi::Knob (parent, _proxy_fade_in, "Fade in", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::FadeIn, 1000), 2);
	_control_fade_in->set_unit_bay (_mikuru->unit_bay());
	_control_frequency = new Haruhi::Knob (parent, _proxy_frequency, "Frequency", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::Frequency, 3000), 3);
	_control_frequency->set_unit_bay (_mikuru->unit_bay());
	_control_level = new Haruhi::Knob (parent, _proxy_level, "Level", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::Level, 100), 2);
	_control_level->set_unit_bay (_mikuru->unit_bay());
	_control_depth = new Haruhi::Knob (parent, _proxy_depth, "Depth", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::Depth, 100), 2);
	_control_depth->set_unit_bay (_mikuru->unit_bay());
	_control_phase = new Haruhi::Knob (parent, _proxy_phase, "Phase", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::Phase, 1000), 3);
	_control_phase->set_unit_bay (_mikuru->unit_bay());
	_control_wave_shape = new Haruhi::Knob (parent, _proxy_wave_shape, "W.shape", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::WaveShape, 1000), 3);
	_control_wave_shape->set_unit_bay (_mikuru->unit_bay());
	_control_fade_out = new Haruhi::Knob (parent, _proxy_fade_out, "Fade out", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::LFO::FadeOut, 100), 2);
	_control_fade_out->set_unit_bay (_mikuru->unit_bay());

	QObject::connect (_control_fade_out, SIGNAL (changed (int)), this, SLOT (update_plot()));
	QObject::connect (_control_wave_shape, SIGNAL (changed (int)), this, SLOT (update_plot()));
	QObject::connect (_control_phase, SIGNAL (changed (int)), this, SLOT (update_plot()));
}


void
LFO::create_widgets (QWidget* knobs_panel)
{
	Params::LFO p = _params;

	QToolTip::add (_control_wave_shape, "Wave shape");
	QToolTip::add (_control_phase, "Start phase");

	QFrame* plot_frame = new QFrame (knobs_panel);
	plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_plot = new Haruhi::WavePlot (plot_frame);
	QVBoxLayout* plot_frame_layout = new QVBoxLayout (plot_frame, 0, Config::spacing);
	plot_frame_layout->addWidget (_plot);

	Q3GroupBox* grid1 = new Q3GroupBox (2, Qt::Horizontal, "", this);
	grid1->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	grid1->setInsideMargin (3 * Config::margin);

	_enabled = new QCheckBox ("Enabled", grid1);
	_enabled->setChecked (p.enabled);
	QObject::connect (_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	_wave_invert = new QCheckBox ("Invert wave", grid1);
	_wave_invert->setChecked (p.wave_invert);
	QObject::connect (_wave_invert, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QObject::connect (_wave_invert, SIGNAL (toggled (bool)), this, SLOT (update_plot()));

	_random_start_phase = new QCheckBox ("Random phase", grid1);
	_random_start_phase->setChecked (p.random_start_phase);
	QObject::connect (_random_start_phase, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	_fade_out_enabled = new QCheckBox ("Fade out", grid1);
	_fade_out_enabled->setChecked (p.fade_out_enabled);
	QObject::connect (_fade_out_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QObject::connect (_fade_out_enabled, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	_tempo_sync = new QCheckBox ("Sync with tempo", grid1);
	_tempo_sync->setChecked (p.tempo_sync);
	QObject::connect (_tempo_sync, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QObject::connect (_tempo_sync, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	QWidget* tempo_grid = new QWidget (grid1);
	QHBoxLayout* tempo_layout = new QHBoxLayout (tempo_grid, 0, Config::spacing);
	tempo_layout->setAutoAdd (true);

	_tempo_numerator = new QSpinBox (1, 64, 1, tempo_grid);
	_tempo_numerator->setValue (p.tempo_numerator);
	QToolTip::add (_tempo_numerator, "Tempo divider numerator");
	QObject::connect (_tempo_numerator, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	_tempo_denominator = new QSpinBox (1, 64, 1, tempo_grid);
	_tempo_denominator->setValue (p.tempo_denominator);
	QToolTip::add (_tempo_denominator, "Tempo divider denonimator");
	QObject::connect (_tempo_denominator, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	new QLabel ("Wave:", grid1);
	_wave_type = new QComboBox (grid1);
	_wave_type->insertItem (Config::Icons16::wave_sine(), "Sine", Params::LFO::Sine);
	_wave_type->insertItem (Config::Icons16::wave_triangle(), "Triangle", Params::LFO::Triangle);
	_wave_type->insertItem (Config::Icons16::wave_square(), "Square", Params::LFO::Square);
	_wave_type->insertItem (Config::Icons16::wave_sawtooth(), "Sawtooth", Params::LFO::Sawtooth);
	_wave_type->insertItem (Config::Icons16::wave_pulse(), "Pulse", Params::LFO::Pulse);
	_wave_type->insertItem (Config::Icons16::wave_random_square(), "Random square", Params::LFO::RandomSquare);
	_wave_type->insertItem (Config::Icons16::wave_random_triangle(), "Random triangle", Params::LFO::RandomTriangle);
	_wave_type->setCurrentItem (p.wave_type);
	QObject::connect (_wave_type, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_wave_type, SIGNAL (activated (int)), this, SLOT (update_plot()));
	QObject::connect (_wave_type, SIGNAL (activated (int)), this, SLOT (update_widgets()));

	new QLabel ("Function:", grid1);
	_function = new QComboBox (grid1);
	_function->insertItem ("Log. E", Params::LFO::LogarithmicE);
	_function->insertItem ("Log. 2", Params::LFO::Logarithmic2);
	_function->insertItem ("Linear", Params::LFO::Linear);
	_function->insertItem ("Exp. 2", Params::LFO::Expotential2);
	_function->insertItem ("Exp. E", Params::LFO::ExpotentialE);
	_function->setCurrentItem (p.function);
	QObject::connect (_function, SIGNAL (activated (int)), this, SLOT (update_params()));

	new QLabel ("Mode:", grid1);
	_mode = new QComboBox (grid1);
	_mode->insertItem ("Polyphonic", Params::LFO::Polyphonic);
	_mode->insertItem ("Common Keysync", Params::LFO::CommonKeySync);
	_mode->insertItem ("Common Continuous", Params::LFO::CommonContinuous);
	_mode->setCurrentItem (p.mode);
	QObject::connect (_mode, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_mode, SIGNAL (activated (int)), this, SLOT (update_widgets()));

	// Layout:

	QVBoxLayout* v1 = new QVBoxLayout (knobs_panel, 0, Config::spacing);
	QHBoxLayout* h1 = new QHBoxLayout (v1, Config::spacing);
	h1->addWidget (_control_fade_in);
	h1->addWidget (_control_frequency);
	h1->addWidget (_control_level);
	h1->addWidget (_control_depth);
	h1->addWidget (_control_fade_out);
	QHBoxLayout* h2 = new QHBoxLayout (v1, Config::spacing);
	h2->addWidget (_control_delay);
	h2->addWidget (plot_frame);
	h2->addWidget (_control_wave_shape);
	h2->addWidget (_control_phase);
	v1->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	QHBoxLayout* h3 = new QHBoxLayout (this, Config::margin, Config::spacing);
	h3->addWidget (knobs_panel);
	QVBoxLayout* v2 = new QVBoxLayout (h3, Config::spacing);
	v2->addWidget (grid1);
	v2->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}


void
LFO::voice_created (VoiceManager*, Voice* voice)
{
	int mode = atomic (_params.mode);
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
		osc->set_fade_out_enabled (atomic (_params.fade_out_enabled));
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
	_proxy_delay->process_events();
	_proxy_fade_in->process_events();
	_proxy_frequency->process_events();
	_proxy_level->process_events();
	_proxy_depth->process_events();
	_proxy_phase->process_events();
	_proxy_wave_shape->process_events();
	_proxy_fade_out->process_events();

	// Skip if disabled or not connected:
	if (!atomic (_params.enabled) || _port_output->forward_connections().empty())
		return;

	int wave_type = atomic (_params.wave_type);
	Core::Timestamp t = _mikuru->graph()->timestamp();
	unsigned int sample_rate = _mikuru->graph()->sample_rate();
	unsigned int buffer_size = _mikuru->graph()->buffer_size();
	DSP::ParametricWave* wave = _waves[wave_type];
	update_wave_param();
	float frequency = _params.frequency.to_f() / sample_rate;
	float level = _params.level.to_f();
	float depth = _params.depth.to_f();
	bool invert = atomic (_params.wave_invert);

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
		_port_output->event_buffer()->push (new Core::VoiceControllerEvent (t, voice->voice_id(), apply_function (osc->advance (buffer_size))));
	}

	int mode = atomic (_params.mode);
	if ((mode == Params::LFO::CommonKeySync && _pressed_keys > 0) || mode == Params::LFO::CommonContinuous)
	{
		_common_osc.set_wave (wave, wave_type == Params::LFO::RandomSquare || wave_type == Params::LFO::RandomTriangle);
		_common_osc.set_frequency (frequency);
		_common_osc.set_level (level);
		_common_osc.set_depth (depth);
		_common_osc.set_invert (invert);
		_port_output->event_buffer()->push (new Core::ControllerEvent (t, apply_function (_common_osc.advance (buffer_size))));
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

	int prev_mode = atomic (_params.mode);

	atomic (_params.enabled) = _enabled->isChecked();
	atomic (_params.wave_type) = _wave_type->currentItem();
	atomic (_params.wave_invert) = _wave_invert->isChecked();
	atomic (_params.function) = _function->currentItem();
	atomic (_params.mode) = _mode->currentItem();
	atomic (_params.tempo_sync) = _tempo_sync->isChecked();
	atomic (_params.tempo_numerator) = _tempo_numerator->value();
	atomic (_params.tempo_denominator) = _tempo_denominator->value();
	atomic (_params.random_start_phase) = _random_start_phase->isChecked();
	atomic (_params.fade_out_enabled) = _fade_out_enabled->isChecked();

	// Knob params are updated automatically using #assign_parameter.

	if (prev_mode != atomic (_params.mode))
		reset_common_osc();
}


void
LFO::update_plot()
{
	bool random = atomic (_params.wave_type) == Params::LFO::RandomSquare || atomic (_params.wave_type) == Params::LFO::RandomTriangle;
	if (!random)
	{
		update_wave_param();
		_plot->assign_wave (_waves[atomic (_params.wave_type)], true, true, atomic (_params.wave_invert));
		_plot->set_phase_marker (true, _params.phase.to_f());
		_plot->plot_shape();
	}
}


void
LFO::update_widgets()
{
	bool random = atomic (_params.wave_type) == Params::LFO::RandomSquare || atomic (_params.wave_type) == Params::LFO::RandomTriangle;
	bool continuous = atomic (_params.mode) == Params::LFO::CommonContinuous;
	_control_delay->setEnabled (!continuous);
	_control_fade_in->setEnabled (!continuous);
	_control_fade_out->setEnabled (!continuous && _params.fade_out_enabled);
	_control_phase->setEnabled (!continuous);
	_control_wave_shape->setEnabled (!random);
	_plot->setEnabled (!random);
	_tempo_numerator->setEnabled (_tempo_sync->isChecked());
	_tempo_denominator->setEnabled (_tempo_sync->isChecked());
}


void
LFO::update_wave_param()
{
	_waves[atomic (_params.wave_type)]->set_param (_params.wave_shape.to_f());
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
	_common_osc.set_fade_out_enabled (atomic (_params.fade_out_enabled));
}


Core::Sample
LFO::apply_function (Core::Sample v) const
{
	switch (atomic (_params.function))
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
	return atomic (_params.random_start_phase)
		? _noise.get()
		: _params.phase.to_f();
}

} // namespace MikuruPrivate

