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

#ifndef HARUHI__PLUGINS__YUKI__PART_OSCILLATOR_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_OSCILLATOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>
#include <vector>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QSlider>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "params.h"
#include "filter_widget.h"


namespace Yuki {

namespace DSP = Haruhi::DSP;
using Haruhi::Unique;

class Part;
class PartWidget;


class PartOscillatorWidget: public QWidget
{
	Q_OBJECT

  public:
	PartOscillatorWidget (QWidget* parent, PartWidget* part_widget, Part* part);

	bool
	is_wave_enabled() const;

  public slots:
	/**
	 * Called by PartWidget.
	 */
	void
	widgets_to_wave_params();

	/**
	 * Called by PartWidget.
	 */
	void
	widgets_to_oscillator_params();

	/**
	 * Called by PartWidget.
	 */
	void
	update_widgets();

	/**
	 * Called by PartWidget.
	 */
	void
	params_to_widgets();

	/**
	 * Called by PartWidget.
	 */
	void
	update_phase_marker (float pos);

	/**
	 * Called by PartWidget.
	 */
	void
	update_wave_plots (DSP::Wave* base_wave, DSP::Wave* final_wave);

  private:
	PartWidget*					_part_widget;
	Part*						_part;

	// Waveform knobs:
	Unique<Haruhi::Knob>		_knob_wave_shape;
	Unique<Haruhi::Knob>		_knob_modulator_amplitude;
	Unique<Haruhi::Knob>		_knob_modulator_index;
	Unique<Haruhi::Knob>		_knob_modulator_shape;

	// Volume knobs:
	Unique<Haruhi::Knob>		_knob_volume;
	Unique<Haruhi::Knob>		_knob_panorama;
	Unique<Haruhi::Knob>		_knob_detune;
	Unique<Haruhi::Knob>		_knob_pitchbend;
	Unique<Haruhi::Knob>		_knob_velocity_sens;

	// Unison knobs:
	Unique<Haruhi::Knob>		_knob_unison_index;
	Unique<Haruhi::Knob>		_knob_unison_spread;
	Unique<Haruhi::Knob>		_knob_unison_init;
	Unique<Haruhi::Knob>		_knob_unison_noise;
	Unique<Haruhi::Knob>		_knob_unison_vibrato_level;
	Unique<Haruhi::Knob>		_knob_unison_vibrato_frequency;

	// Other knobs:
	Unique<Haruhi::Knob>		_knob_portamento_time;
	Unique<Haruhi::Knob>		_knob_phase;
	Unique<Haruhi::Knob>		_knob_noise_level;

	// Waveform-related:
	QComboBox*					_wave_type;
	QComboBox*					_modulator_type;
	QComboBox*					_modulator_wave_type;
	Unique<Haruhi::WavePlot>	_base_wave_plot;
	Unique<Haruhi::WavePlot>	_final_wave_plot;

	// Pitchbend/transposition:
	QPushButton*				_const_portamento_time;
	QPushButton*				_pitchbend_enabled;
	QSpinBox*					_pitchbend_up_semitones;
	QSpinBox*					_pitchbend_down_semitones;
	QSpinBox*					_transposition_semitones;
	QSpinBox*					_frequency_modulation_range;

	QPushButton*				_unison_stereo;
	QPushButton*				_pseudo_stereo;
	QPushButton*				_auto_center;
	QPushButton*				_wave_enabled;
	QPushButton*				_noise_enabled;
	FilterWidget*				_filter_1;
	FilterWidget*				_filter_2;
	QComboBox*					_filter_configuration;
};

} // namespace Yuki

#endif

