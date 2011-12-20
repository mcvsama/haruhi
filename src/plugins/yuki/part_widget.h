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
#include <haruhi/dsp/wave.h>
#include <haruhi/utility/signal.h>

// Local:
#include "part.h"
#include "filter_widget.h"


namespace Yuki {

namespace DSP = Haruhi::DSP;

class Part;
class PartManagerWidget;
class Slider;

class PartWidget:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

	typedef std::vector<QSlider*>		Sliders;
	typedef std::vector<QPushButton*>	Buttons;

  public:
	PartWidget (PartManagerWidget*, Part*);

	~PartWidget();

	/**
	 * Link to associated Part object.
	 */
	Part*
	part() const;

  private slots:
	/**
	 * Called when wave params change.
	 * \entry	UI thread
	 */
	void
	wave_params_updated();

	/**
	 * Called when oscillator params change.
	 */
	void
	oscillator_params_updated();

	/**
	 * Called when 'Harmonics' button is clicked.
	 * \entry	UI thread
	 */
	void
	show_harmonics();

	/**
	 * Update phase marker on wave plots.
	 * \entry	UI thread
	 */
	void
	update_phase_marker();

	/**
	 * Update widgets deps (enable/disable, etc.)
	 *
	 */
	void
	update_widgets();

  private:
	/**
	 * Highlights selected button.
	 */
	void
	set_button_highlighted (QPushButton* button, bool highlight);

	/**
	 * Connected to part's waves_updated signal.
	 * \entry	UI thread
	 */
	void
	update_wave_plots();

	/**
	 * Call update_wave_plots() from UI thread later.
	 */
	void
	post_update_wave_plots();

  private:
	PartManagerWidget*	_part_manager_widget;
	Part*				_part;
	DSP::Wave*			_cached_final_wave;

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
	Haruhi::Knob*		_knob_unison_vibrato_level;
	Haruhi::Knob*		_knob_unison_vibrato_frequency;

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
	QWidget*			_harmonics_widget;
	QWidget*			_harmonic_phases_widget;
	QDialog*			_harmonics_window;

	// Pitchbend/transposition:
	QPushButton*		_const_portamento_time;
	QPushButton*		_pitchbend_enabled;
	QSpinBox*			_pitchbend_up_semitones;
	QSpinBox*			_pitchbend_down_semitones;
	QSpinBox*			_transposition_semitones;
	QSpinBox*			_frequency_modulation_range;

	// Other:
	QWidget*			_panel;
	QPushButton*		_auto_center;
	QPushButton*		_unison_stereo;
	QPushButton*		_pseudo_stereo;
	QCheckBox*			_part_enabled;
	QPushButton*		_wave_enabled;
	QPushButton*		_noise_enabled;
	QColor				_std_button_bg;
	QColor				_std_button_fg;
	FilterWidget*		_filter_1;
	FilterWidget*		_filter_2;
	QComboBox*			_filter_configuration;
};


/**
 * Slider that implements slot reset() which
 * sets slider value to 0.
 */
class Slider: public QSlider
{
	Q_OBJECT

  public:
	Slider (int min_value, int max_value, int page_step, int value, Qt::Orientation orientation, QWidget* parent);

  public slots:
	/**
	 * Set slider value to 0.
	 */
	void
	reset();
};


inline Part*
PartWidget::part() const
{
	return _part;
}


inline void
PartWidget::set_button_highlighted (QPushButton* button, bool highlight)
{
	button->setPaletteBackgroundColor (highlight ? QColor (0x00, 0xff, 0x00) : _std_button_bg);
	button->setPaletteForegroundColor (highlight ? QColor (0x00, 0x00, 0x00) : _std_button_fg);
}


inline
Slider::Slider (int min_value, int max_value, int page_step, int value, Qt::Orientation orientation, QWidget* parent):
	QSlider (orientation, parent)
{
	setMinimum (min_value);
	setMaximum (max_value);
	setPageStep (page_step);
	setValue (value);
}


inline void
Slider::reset()
{
	setValue (0);
}

} // namespace Yuki

#endif

