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

#ifndef HARUHI__PLUGINS__YUKI__PART_WIDGET_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QDialog>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/widgets/wave_plot.h>


namespace Yuki {

class Part;
class PartManagerWidget;

class PartWidget: public QWidget
{
	Q_OBJECT

	typedef std::vector<QSlider*>		Sliders;
	typedef std::vector<QPushButton*>	Buttons;

  public:
	PartWidget (PartManagerWidget*, Part*);

	~PartWidget();

	Part*
	part() const;

  private:
	PartManagerWidget*	_part_manager_widget;
	Part*				_part;

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
	Sliders				_harmonic_phases_sliders;
	Buttons				_harmonic_phases_resets;
	QColor				_std_button_bg;
	QColor				_std_button_fg;
	QWidget*			_harmonics_tab;
	QWidget*			_harmonic_phases_tab;
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


inline Part*
PartWidget::part() const
{
	return _part;
}

} // namespace Yuki

#endif

