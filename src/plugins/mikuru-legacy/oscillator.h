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

#ifndef HARUHI__PLUGINS__MIKURU__OSCILLATOR_H__INCLUDED
#define HARUHI__PLUGINS__MIKURU__OSCILLATOR_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QSlider>
#include <QtGui/QPushButton>
#include <QtGui/QDialog>

// Haruhi:
#include <haruhi/dsp/wave.h>
#include <haruhi/dsp/wavetable.h>
#include <haruhi/dsp/functions.h>
#include <haruhi/dsp/harmonics_wave.h>
#include <haruhi/dsp/modulated_wave.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/graph/port_group.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/utility/signal.h>
#include <haruhi/utility/memory.h>

// Local:
#include "event_dispatcher.h"
#include "widgets.h"
#include "params.h"


class Mikuru;

namespace Haruhi {
	class WavePlot;
}

namespace MikuruPrivate {

namespace DSP = Haruhi::DSP;
class Part;
class WaveComputer;

/**
 * Slider that implements slot reset() which
 * sets slider value to 0.
 */
class Slider: public QSlider
{
	Q_OBJECT

  public:
	Slider (int min_value, int max_value, int page_step, int value, Qt::Orientation orientation, QWidget* parent):
		QSlider (orientation, parent)
	{
		setMinimum (min_value);
		setMaximum (max_value);
		setPageStep (page_step);
		setValue (value);
	}

  public slots:
	/**
	 * Sets slider value to 0.
	 */
	void
	reset() { setValue (0); }
};


class Oscillator:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

	friend class Patch;

	/**
	 * Holds info about wave (name, icon, pointer to parametric wave).
	 * Instead of having multiple lists, we pack such data into WaveInfo for convenience.
	 */
	struct WaveInfo
	{
		WaveInfo():
			wave (0)
		{ }

		/**
		 * Takes ownership of wave - deletes it upon destruction.
		 */
		WaveInfo (QPixmap const& icon, QString const& name, DSP::ParametricWave* wave):
			icon (icon),
			name (name),
			wave (wave)
		{ }

		QPixmap						icon;
		QString						name;
		Shared<DSP::ParametricWave>	wave;
	};

	typedef std::vector<WaveInfo>		Waves;
	typedef std::vector<QSlider*>		Sliders;
	typedef std::vector<QPushButton*>	Buttons;

  public:
	Oscillator (Part* part, Haruhi::PortGroup* port_group, QString const& port_prefix, Mikuru* mikuru, QWidget* parent);

	~Oscillator();

	/**
	 * \returns	Waveform params.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	Params::Waveform*
	waveform_params() { return &_waveform_params; }

	Params::Oscillator*
	oscillator_params() { return &_oscillator_params; }

	Params::Voice*
	voice_params() { return &_voice_params; }

	/**
	 * \entry	Only from Engine thread.
	 */
	void
	process_events();

	Haruhi::EventPort*
	pitchbend_port() const { return _port_pitchbend; }

	Haruhi::EventPort*
	amplitude_port() const { return _port_amplitude; }

	Haruhi::EventPort*
	frequency_port() const { return _port_frequency; }

	/**
	 * \returns	currently used Wavetable.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	DSP::Wavetable*
	wavetable();

	/**
	 * Switches double-buffered wavetable in WaveComputer.
	 */
	void
	switch_wavetables();

  public slots:
	/**
	 * Loads widgets values from Params struct.
	 * \entry	Only from UI thread.
	 */
	void
	load_waveform_params();

	void
	load_oscillator_params();

	void
	load_voice_params();

	/**
	 * Loads params from given struct and updates widgets.
	 * \entry	Only from UI thread.
	 */
	void
	load_waveform_params (Params::Waveform& params);

	void
	load_oscillator_params (Params::Oscillator& params);

	void
	load_voice_params (Params::Voice& params);

	/**
	 * Updates Params structure from widgets.
	 * \entry	Only from UI thread.
	 */
	void
	update_waveform_params();

	void
	update_oscillator_params();

	/**
	 * Updates widgets.
	 */
	void
	update_widgets();

  private slots:
	/**
	 * Called when any value or phase slider is moved.
	 * \entry	Only from UI thread.
	 */
	void
	sliders_updated();

	void
	show_harmonics();

	/**
	 * Updates phase marker on wave plots.
	 */
	void
	update_phase_marker();

  private:
	/**
	 * Sets up params callbacks.
	 */
	void
	setup_params();

	void
	update_voice_panorama();

	void
	update_voice_detune();

	void
	update_voice_pitchbend();

	void
	update_voice_unison_index();

	void
	update_voice_unison_spread();

	void
	update_voice_unison_init();

	void
	update_voice_unison_noise();

	void
	update_voice_velocity_sens();

	/**
	 * Recomputes wave.
	 * \entry	Only from UI thread.
	 */
	void
	recompute_wave();

	/**
	 * Returns WaveInfo for currently selected wave in UI.
	 */
	WaveInfo&
	active_wave();

	/**
	 * Returns WaveInfo for currently selected modulator wave in UI.
	 */
	WaveInfo&
	active_modulator_wave();

	/**
	 * Redraws wave plot.
	 * \entry	Only from WaveComputer thread.
	 */
	void
	update_wave_plot (Shared<DSP::Wave> const& wave);

	/**
	 * Higlights or resets button color.
	 * \entry	Only from UI thread.
	 */
	void
	set_button_highlighted (QPushButton* button, bool highlight);

  private:
	Mikuru*				_mikuru;
	Part*				_part;
	Params::Waveform	_waveform_params;
	Params::Oscillator	_oscillator_params;
	Params::Voice		_voice_params;
	bool				_loading_params;
	Waves				_waves;
	Waves				_modulator_waves;
	WaveComputer*		_wave_computer;
	Shared<DSP::Wave>	_plotters_wave;

	// Waveform ports:
	Haruhi::EventPort*	_port_wave_shape;
	Haruhi::EventPort*	_port_modulator_amplitude;
	Haruhi::EventPort*	_port_modulator_index;
	Haruhi::EventPort*	_port_modulator_shape;

	// Part ports:
	Haruhi::EventPort*	_port_volume;
	Haruhi::EventPort*	_port_portamento_time;
	Haruhi::EventPort*	_port_phase;

	// Polyphonic-input ports:
	Haruhi::EventPort*	_port_amplitude;
	Haruhi::EventPort*	_port_frequency;
	Haruhi::EventPort*	_port_panorama;
	Haruhi::EventPort*	_port_detune;
	Haruhi::EventPort*	_port_pitchbend;
	Haruhi::EventPort*	_port_velocity_sens;
	Haruhi::EventPort*	_port_unison_index;
	Haruhi::EventPort*	_port_unison_spread;
	Haruhi::EventPort*	_port_unison_init;
	Haruhi::EventPort*	_port_unison_noise;
	Haruhi::EventPort*	_port_noise_level;

	// Event dispatchers for polyphonic-input ports:
	EventDispatcher*	_evdisp_amplitude;
	EventDispatcher*	_evdisp_frequency;
	EventDispatcher*	_evdisp_panorama;
	EventDispatcher*	_evdisp_detune;
	EventDispatcher*	_evdisp_pitchbend;
	EventDispatcher*	_evdisp_velocity_sens;
	EventDispatcher*	_evdisp_unison_index;
	EventDispatcher*	_evdisp_unison_spread;
	EventDispatcher*	_evdisp_unison_init;
	EventDispatcher*	_evdisp_unison_noise;

	// Waveform knobs:
	Haruhi::Knob*		_knob_wave_shape;
	Haruhi::Knob*		_knob_modulator_amplitude;
	Haruhi::Knob*		_knob_modulator_index;
	Haruhi::Knob*		_knob_modulator_shape;

	// Volume knobs:
	Haruhi::Knob*		_knob_volume;
	Haruhi::Knob*		_knob_panorama;
	Haruhi::Knob*		_knob_detune;
	Haruhi::Knob*		_knob_pitchbend;
	Haruhi::Knob*		_knob_velocity_sens;

	// Unison knobs:
	Haruhi::Knob*		_knob_unison_index;
	Haruhi::Knob*		_knob_unison_spread;
	Haruhi::Knob*		_knob_unison_init;
	Haruhi::Knob*		_knob_unison_noise;

	// Other knobs:
	Haruhi::Knob*		_knob_portamento_time;
	Haruhi::Knob*		_knob_phase;
	Haruhi::Knob*		_knob_noise_level;

	// Waveform-related:
	Haruhi::WavePlot*	_base_wave_plot;
	Haruhi::WavePlot*	_final_wave_plot;
	QComboBox*			_wave_type;
	QComboBox*			_modulator_type;
	QComboBox*			_modulator_wave_type;
	Sliders				_harmonics_sliders;
	Buttons				_harmonics_resets;
	Sliders				_phases_sliders;
	Buttons				_phases_resets;
	QColor				_std_button_bg;
	QColor				_std_button_fg;
	QWidget*			_harmonics_tab;
	QWidget*			_phases_tab;
	QDialog*			_harmonics_window;
	QTabWidget*			_harmonics_and_phases_tabs;

	// Pitchbend/transposition:
	QCheckBox*			_const_portamento_time;
	QCheckBox*			_pitchbend_enabled;
	QCheckBox*			_pitchbend_released;
	QSpinBox*			_pitchbend_up_semitones;
	QSpinBox*			_pitchbend_down_semitones;
	QSpinBox*			_transposition_semitones;
	QSpinBox*			_frequency_modulation_range;

	// Monophonic:
	QCheckBox*			_monophonic;
	QCheckBox*			_monophonic_retrigger;
	QComboBox*			_monophonic_key_priority;

	// Other:
	QCheckBox*			_unison_stereo;
	QCheckBox*			_pseudo_stereo;
	QPushButton*		_wave_enabled;
	QPushButton*		_noise_enabled;
};

} // namespace MikuruPrivate

#endif

