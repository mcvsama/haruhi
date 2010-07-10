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
#include <string>

// Qt:
#include <QtGui/QToolTip>
#include <QtGui/QTabWidget>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3Grid>

// Haruhi:
#include <haruhi/controller_proxy.h>
#include <haruhi/dsp/functions.h>
#include <haruhi/dsp/wavetable.h>
#include <haruhi/dsp/harmonics_wave.h>
#include <haruhi/dsp/modulated_wave.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/widgets/wave_plot.h>
#include <haruhi/utility/memory.h>

// Local:
#include "mikuru.h"
#include "waveform.h"
#include "widgets.h"
#include "wave_computer.h"


namespace MikuruPrivate {

Waveform::Waveform (Part* part, Core::PortGroup* port_group, QString const& q_port_prefix, Mikuru* mikuru, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_part (part),
	_loading_params (false)
{
	std::string port_prefix = q_port_prefix.toStdString();

	_mikuru->graph()->lock();
	_port_wave_shape = new Core::EventPort (_mikuru, port_prefix + " - Wave shape", Core::Port::Input, port_group);
	_port_modulator_amplitude = new Core::EventPort (_mikuru, port_prefix + " - Modulator amplitude", Core::Port::Input, port_group);
	_port_modulator_index = new Core::EventPort (_mikuru, port_prefix + " - Modulator index", Core::Port::Input, port_group);
	_port_modulator_shape = new Core::EventPort (_mikuru, port_prefix + " - Modulator shape", Core::Port::Input, port_group);
	_mikuru->graph()->unlock();

	Params::Waveform p = _params;
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	// Wave types:

	_waves.push_back (WaveInfo (Config::Icons16::wave_sine(),		"Sine",		new DSP::ParametricWaves::Sine()));
	_waves.push_back (WaveInfo (Config::Icons16::wave_triangle(),	"Triangle",	new DSP::ParametricWaves::Triangle()));
	_waves.push_back (WaveInfo (Config::Icons16::wave_square(),		"Square",	new DSP::ParametricWaves::Square()));
	_waves.push_back (WaveInfo (Config::Icons16::wave_sawtooth(),	"Sawtooth",	new DSP::ParametricWaves::Sawtooth()));
	_waves.push_back (WaveInfo (Config::Icons16::wave_pulse(),		"Pulse",	new DSP::ParametricWaves::Pulse()));
	_waves.push_back (WaveInfo (Config::Icons16::wave_power(),		"Power",	new DSP::ParametricWaves::Power()));
	_waves.push_back (WaveInfo (Config::Icons16::wave_gauss(),		"Gauss",	new DSP::ParametricWaves::Gauss()));
	_waves.push_back (WaveInfo (Config::Icons16::wave_diode(),		"Diode",	new DSP::ParametricWaves::Diode()));
	_waves.push_back (WaveInfo (Config::Icons16::wave_chirp(),		"Chirp",	new DSP::ParametricWaves::Chirp()));

	// Modulator waves:

	_modulator_waves.push_back (WaveInfo (Config::Icons16::wave_sine(),		"Sine",		new DSP::ParametricWaves::Sine()));
	_modulator_waves.push_back (WaveInfo (Config::Icons16::wave_triangle(),	"Triangle",	new DSP::ParametricWaves::Triangle()));
	_modulator_waves.push_back (WaveInfo (Config::Icons16::wave_square(),	"Square",	new DSP::ParametricWaves::Square()));
	_modulator_waves.push_back (WaveInfo (Config::Icons16::wave_sawtooth(),	"Sawtooth",	new DSP::ParametricWaves::Sawtooth()));

	_wave_computer = new WaveComputer();
	_wave_computer->finished.connect (this, &Waveform::update_wave_plot);

	// Wave plot:

	QFrame* top_frame = new QFrame (this);
	top_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

	QFrame* base_plot_frame = new QFrame (top_frame);
	base_plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	base_plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_base_wave_plot = new WavePlot (base_plot_frame);
	QToolTip::add (_base_wave_plot, "Base wave");
	QVBoxLayout* base_plot_frame_layout = new QVBoxLayout (base_plot_frame, 0, Config::spacing);
	base_plot_frame_layout->addWidget (_base_wave_plot);

	QFrame* harmonics_plot_frame = new QFrame (top_frame);
	harmonics_plot_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	harmonics_plot_frame->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_final_wave_plot = new WavePlot (harmonics_plot_frame);
	QToolTip::add (_final_wave_plot, "Wave with harmonics and modulation");
	QVBoxLayout* harmonics_plot_frame_layout = new QVBoxLayout (harmonics_plot_frame, 0, Config::spacing);
	harmonics_plot_frame_layout->addWidget (_final_wave_plot);

	// Wave type:

	_wave_type = new QComboBox (top_frame);
	for (Waves::size_type i = 0; i < _waves.size(); ++i)
		_wave_type->insertItem (_waves[i].icon, _waves[i].name, i);
	_wave_type->setCurrentItem (p.wave_type);
	QObject::connect (_wave_type, SIGNAL (activated (int)), this, SLOT (update_params()));
	QToolTip::add (_wave_type, "Oscillator wave type");

	// Modulator type:

	_modulator_type = new QComboBox (top_frame);
	_modulator_type->insertItem (Config::Icons16::modulator_ring(), "Ring", DSP::ModulatedWave::Ring);
	_modulator_type->insertItem (Config::Icons16::modulator_fm(), "FM", DSP::ModulatedWave::Frequency);
	_modulator_type->setCurrentItem (p.modulator_type);
	QObject::connect (_modulator_type, SIGNAL (activated (int)), this, SLOT (update_params()));
	QToolTip::add (_modulator_type, "Modulator type");

	// Modulator wave type:

	_modulator_wave_type = new QComboBox (top_frame);
	for (Waves::size_type i = 0; i < _modulator_waves.size(); ++i)
		_modulator_wave_type->insertItem (_modulator_waves[i].icon, _modulator_waves[i].name, i);
	_modulator_wave_type->setCurrentItem (p.modulator_wave_type);
	QObject::connect (_modulator_wave_type, SIGNAL (activated (int)), this, SLOT (update_params()));
	QToolTip::add (_modulator_wave_type, "Modulator wave type");

	// Wave parameters:

	_proxy_wave_shape = new ControllerProxy (_port_wave_shape, &_params.wave_shape, 0, HARUHI_MIKURU_MINMAX (Params::Waveform::WaveShape), p.wave_shape);
	_proxy_modulator_amplitude = new ControllerProxy (_port_modulator_amplitude, &_params.modulator_amplitude, 0, HARUHI_MIKURU_MINMAX (Params::Waveform::ModulatorAmplitude), p.modulator_amplitude);
	_proxy_modulator_index = new ControllerProxy (_port_modulator_index, &_params.modulator_index, 0, HARUHI_MIKURU_MINMAX (Params::Waveform::ModulatorIndex), p.modulator_index);
	_proxy_modulator_shape = new ControllerProxy (_port_modulator_shape, &_params.modulator_shape, 0, HARUHI_MIKURU_MINMAX (Params::Waveform::ModulatorShape), p.modulator_shape);

	_control_wave_shape = new Knob (top_frame, _proxy_wave_shape, "Shape", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Waveform::WaveShape, 100), 2);
	_control_wave_shape->set_unit_bay (_mikuru->unit_bay());
	_control_modulator_amplitude = new Knob (top_frame, _proxy_modulator_amplitude, "Mod.amp.", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Waveform::ModulatorAmplitude, 100), 2);
	_control_modulator_amplitude->set_unit_bay (_mikuru->unit_bay());
	_control_modulator_index = new Knob (top_frame, _proxy_modulator_index, "Mod.index", HARUHI_MIKURU_PARAMS_FOR_KNOB (Params::Waveform::ModulatorIndex), 1, 0);
	_control_modulator_index->set_unit_bay (_mikuru->unit_bay());
	_control_modulator_shape = new Knob (top_frame, _proxy_modulator_shape, "Mod.shape", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Waveform::ModulatorShape, 100), 2);
	_control_modulator_shape->set_unit_bay (_mikuru->unit_bay());

	QObject::connect (_control_wave_shape, SIGNAL (changed (int)), this, SLOT (recompute_wave()));
	QObject::connect (_control_modulator_amplitude, SIGNAL (changed (int)), this, SLOT (recompute_wave()));
	QObject::connect (_control_modulator_index, SIGNAL (changed (int)), this, SLOT (recompute_wave()));
	QObject::connect (_control_modulator_shape, SIGNAL (changed (int)), this, SLOT (recompute_wave()));

	// Tabs:

	QTabWidget* harmonics_and_phases_tabs = new QTabWidget (this);

	// Harmonics:

	QWidget* harmonics_tab = new QWidget (harmonics_and_phases_tabs);
	harmonics_tab->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QWidget* harmonics_grid = new QWidget (harmonics_tab);
	QGridLayout* harmonics_layout = new QGridLayout (harmonics_grid);
	harmonics_layout->setSpacing (0);
	for (Sliders::size_type i = 0; i < Params::Waveform::HarmonicsNumber; ++i)
	{
		harmonics_layout->setColumnMinimumWidth (i, 0);

		int def = i == 0 ? Params::Waveform::HarmonicMax : Params::Waveform::HarmonicDefault;
		Slider* slider = new Slider (Params::Waveform::HarmonicMin, Params::Waveform::HarmonicMax, Params::Waveform::HarmonicMax / 20, def, Qt::Vertical, harmonics_grid);
		slider->setTracking (true);
		QObject::connect (slider, SIGNAL (valueChanged (int)), this, SLOT (sliders_updated()));
		harmonics_layout->addWidget (slider, 0, i);

		QLabel* label = new QLabel (QString ("%1").arg (i + 1), harmonics_grid);
		label->setAlignment (Qt::AlignCenter);
		label->setFixedWidth (23);
		harmonics_layout->addWidget (label, 1, i);

		QPushButton* reset = new QPushButton ("R", harmonics_grid);
		reset->setFixedWidth (23);
		reset->setFixedHeight (18);
		QToolTip::add (reset, "Reset");
		QObject::connect (reset, SIGNAL (clicked()), slider, SLOT (reset()));
		harmonics_layout->addWidget (reset, 2, i);

		_harmonics_sliders.push_back (slider);
		_harmonics_resets.push_back (reset);
	}
	QHBoxLayout* harmonics_tab_layout = new QHBoxLayout (harmonics_tab, 0, 0);
	harmonics_tab_layout->addWidget (harmonics_grid);

	// Phases:

	QWidget* phases_tab = new QWidget (harmonics_and_phases_tabs);
	phases_tab->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QWidget* phases_grid = new QWidget (phases_tab);
	QGridLayout* phases_layout = new QGridLayout (phases_grid);
	phases_layout->setSpacing (0);
	for (Sliders::size_type i = 0; i < Params::Waveform::HarmonicsNumber; ++i)
	{
		harmonics_layout->setColumnMinimumWidth (i, 0);

		Slider* slider = new Slider (Params::Waveform::PhaseMin, Params::Waveform::PhaseMax, Params::Waveform::PhaseMax / 20, Params::Waveform::PhaseDefault, Qt::Vertical, phases_grid);
		slider->setTracking (true);
		QObject::connect (slider, SIGNAL (valueChanged (int)), this, SLOT (sliders_updated()));
		phases_layout->addWidget (slider, 0, i);

		QLabel* label = new QLabel (QString ("%1").arg (i + 1), phases_grid);
		label->setAlignment (Qt::AlignCenter);
		label->setFixedWidth (23);
		phases_layout->addWidget (label, 1, i);

		QPushButton* reset = new QPushButton ("R", phases_grid);
		reset->setFixedWidth (23);
		reset->setFixedHeight (18);
		QToolTip::add (reset, "Reset");
		phases_layout->addWidget (reset, 2, i);

		QObject::connect (reset, SIGNAL (clicked()), slider, SLOT (reset()));

		_phases_sliders.push_back (slider);
		_phases_resets.push_back (reset);
	}
	QHBoxLayout* phases_tab_layout = new QHBoxLayout (phases_tab, 0, 0);
	phases_tab_layout->addWidget (phases_grid);

	harmonics_and_phases_tabs->addTab (harmonics_tab, "Harmonics");
	harmonics_and_phases_tabs->addTab (phases_tab, "Phases");

	// Layouts:

	QHBoxLayout* hor1_layout = new QHBoxLayout (top_frame, 0, Config::spacing);
	hor1_layout->addWidget (base_plot_frame);
	hor1_layout->addWidget (harmonics_plot_frame);
	QVBoxLayout* ver1_layout = new QVBoxLayout (hor1_layout, Config::spacing);
	QHBoxLayout* hor2_layout = new QHBoxLayout (ver1_layout, Config::spacing);
	hor2_layout->addWidget (_wave_type);
	hor2_layout->addWidget (_modulator_type);
	hor2_layout->addWidget (_modulator_wave_type);
	QHBoxLayout* hor3_layout = new QHBoxLayout (ver1_layout, Config::spacing);
	hor3_layout->addWidget (_control_wave_shape);
	hor3_layout->addWidget (_control_modulator_amplitude);
	hor3_layout->addWidget (_control_modulator_index);
	hor3_layout->addWidget (_control_modulator_shape);
	hor3_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	QVBoxLayout* layout = new QVBoxLayout (this, Config::margin, Config::spacing);
	layout->addWidget (top_frame);
	layout->addWidget (harmonics_and_phases_tabs);

	_std_button_bg = _harmonics_resets[0]->paletteBackgroundColor();
	_std_button_fg = _harmonics_resets[0]->paletteForegroundColor();

	update_params();
}


Waveform::~Waveform()
{
	// Delete knobs before ControllerProxies:
	delete _control_wave_shape;
	delete _control_modulator_amplitude;
	delete _control_modulator_index;
	delete _control_modulator_shape;

	delete _proxy_wave_shape;
	delete _proxy_modulator_amplitude;
	delete _proxy_modulator_index;
	delete _proxy_modulator_shape;

	_mikuru->graph()->lock();
	delete _wave_computer;
	delete _port_wave_shape;
	delete _port_modulator_amplitude;
	delete _port_modulator_index;
	delete _port_modulator_shape;
	_mikuru->graph()->unlock();
}


void
Waveform::process_events()
{
	_proxy_wave_shape->process_events();
	_proxy_modulator_amplitude->process_events();
	_proxy_modulator_index->process_events();
	_proxy_modulator_shape->process_events();
}


DSP::Wavetable*
Waveform::wavetable()
{
	return _wave_computer->wavetable();
}


void
Waveform::sliders_updated()
{
	if (!_loading_params)
		update_params();
}


void
Waveform::load_params()
{
	Params::Waveform p = _params;
	_loading_params = true;

	_wave_type->setCurrentItem (p.wave_type);
	_modulator_type->setCurrentItem (p.modulator_type);
	_modulator_wave_type->setCurrentItem (p.modulator_wave_type);
	_proxy_wave_shape->set_value (p.wave_shape);
	_proxy_modulator_amplitude->set_value (p.modulator_amplitude);
	_proxy_modulator_index->set_value (p.modulator_index);
	_proxy_modulator_shape->set_value (p.modulator_shape);
	for (Sliders::size_type i = 0; i < _harmonics_sliders.size(); ++i)
		_harmonics_sliders[i]->setValue (p.harmonics[i]);
	for (Sliders::size_type i = 0; i < _phases_sliders.size(); ++i)
		_phases_sliders[i]->setValue (p.phases[i]);

	_loading_params = false;
	update_widgets();
	recompute_wave();
}


void
Waveform::load_params (Params::Waveform& params)
{
	_params = params;
	load_params();
}


void
Waveform::update_params()
{
	if (_loading_params)
		return;

	Params::Waveform p;
	p.wave_type = _wave_type->currentItem();
	p.modulator_type = _modulator_type->currentItem();
	p.modulator_wave_type = _modulator_wave_type->currentItem();
	for (Sliders::size_type i = 0; i < _harmonics_sliders.size(); ++i)
		p.harmonics[i] = _harmonics_sliders[i]->value();
	for (Sliders::size_type i = 0; i < _phases_sliders.size(); ++i)
		p.phases[i] = _phases_sliders[i]->value();
	_params.set_non_controller_params (p);
	recompute_wave();

	// Knob params are updated automatically using #assign_parameter.
}


void
Waveform::update_widgets()
{
}


void
Waveform::update_wave_plot (Shared<DSP::Wave> const& wave)
{
	// Temporarily prevent plotters to use previous wave,
	// because it will be dropped now:
	_base_wave_plot->assign_wave (0);
	_final_wave_plot->assign_wave (0);

	// Replace wave for plotters (will delete previous waves):
	_plotters_wave = wave;

	DSP::ModulatedWave* modulated_wave = dynamic_cast<DSP::ModulatedWave*> (&*_plotters_wave);
	if (modulated_wave)
	{
		DSP::HarmonicsWave* harmonics_wave = dynamic_cast<DSP::HarmonicsWave*> (modulated_wave->wave());
		if (harmonics_wave)
		{
			DSP::ParametricWave* parametric_wave = dynamic_cast<DSP::ParametricWave*> (harmonics_wave->wave());
			if (parametric_wave)
			{
				// We're not Qt-thread, we must use post_plot_shape.

				_base_wave_plot->assign_wave (parametric_wave, false, true);
				_base_wave_plot->post_plot_shape();

				_final_wave_plot->assign_wave (modulated_wave, false, true);
				_final_wave_plot->post_plot_shape();
			}
		}
	}
}


void
Waveform::recompute_wave()
{
	DSP::ParametricWave* pw = _waves[std::min (_params.wave_type, static_cast<unsigned int> (_waves.size() - 1))].wave.get();
	DSP::ParametricWave* mw = _modulator_waves[std::min (_params.modulator_wave_type, static_cast<unsigned int> (_modulator_waves.size() - 1))].wave.get();
	pw->set_param (1.0f * atomic (_params.wave_shape) / Params::Waveform::WaveShapeDenominator);
	mw->set_param (1.0f * atomic (_params.modulator_shape) / Params::Waveform::ModulatorShapeDenominator);
	// Add harmonics to pw:
	DSP::HarmonicsWave* hw = new DSP::HarmonicsWave (pw);
	for (DSP::HarmonicsWave::Harmonics::size_type i = 0; i < hw->harmonics().size(); ++i)
	{
		int hv = _harmonics_sliders[i]->value();
		int pv = _phases_sliders[i]->value();
		float h = 1.0f * hv / Params::Waveform::HarmonicDenominator;
		float p = 1.0f * pv / Params::Waveform::PhaseDenominator;
		// Apply exponential curve to harmonic value:
		h = h > 0 ? std::pow (h, M_E) : -std::pow (-h, M_E);
		hw->set_harmonic (i, h, p);
		set_button_highlighted (_harmonics_resets[i], hv != 0);
		set_button_highlighted (_phases_resets[i], pv != 0);
	}
	// Modulate wave:
	int xt = atomic (_params.modulator_type);
	DSP::ModulatedWave* xw = new DSP::ModulatedWave (hw, mw, static_cast<DSP::ModulatedWave::Type> (xt),
													 1.0f * atomic (_params.modulator_amplitude) / Params::Waveform::ModulatorAmplitudeMax,
													 atomic (_params.modulator_index), true);
	// Recompute:
	_wave_computer->update (xw);
}


void
Waveform::set_button_highlighted (QPushButton* button, bool highlight)
{
	button->setPaletteBackgroundColor (highlight ? QColor (0x00, 0xff, 0x00) : _std_button_bg);
	button->setPaletteForegroundColor (highlight ? QColor (0x00, 0x00, 0x00) : _std_button_fg);
}

} // namespace MikuruPrivate

