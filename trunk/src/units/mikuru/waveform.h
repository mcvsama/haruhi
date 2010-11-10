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

#ifndef HARUHI__UNITS__MIKURU__WAVEFORM_H__INCLUDED
#define HARUHI__UNITS__MIKURU__WAVEFORM_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>
#include <map>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QSlider>
#include <QtGui/QPushButton>
#include <Qt3Support/Q3ScrollView>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/core/port_group.h>
#include <haruhi/core/event_port.h>
#include <haruhi/dsp/wavetable.h>
#include <haruhi/dsp/functions.h>
#include <haruhi/dsp/harmonics_wave.h>
#include <haruhi/dsp/modulated_wave.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/utility/signal.h>
#include <haruhi/utility/memory.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "widgets.h"
#include "params.h"


class Mikuru;

namespace Haruhi {
	class WavePlot;
}


namespace MikuruPrivate {

namespace Core = Haruhi::Core;
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


/**
 * Waveform page.
 * Gives access to waveform parameters (wave type, wave modulation, harmonics, phases, etc).
 * Also holds wavetables and recomputes them by using WaveComputer.
 */
class Waveform:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

  public:
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
	/**
	 * \param	port_prefix is prefix added to all Waveform port names.
	 * \entry	Only UI thread.
	 */
	Waveform (Part* part, Core::PortGroup* port_group, QString const& port_prefix, Mikuru* mikuru, QWidget* parent);

	~Waveform();

	/**
	 * \returns	Waveform params.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	Params::Waveform*
	params() { return &_params; }

	/**
	 * \entry	Only from processing thread.
	 */
	void
	process_events();

	/**
	 * \returns	currently used Wavetable.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	DSP::Wavetable*
	wavetable();

  public slots:
	/**
	 * Called when any value or phase slider is moved.
	 * \entry	Only from UI thread.
	 */
	void
	sliders_updated();

	/**
	 * Loads widgets values from Params struct.
	 * \entry	Only from UI thread.
	 */
	void
	load_params();

	/**
	 * Loads params from given struct and updates widgets.
	 * \entry	Only from UI thread.
	 */
	void
	load_params (Params::Waveform& params);

	/**
	 * Updates Params structure from widgets.
	 * \entry	Only from UI thread.
	 */
	void
	update_params();

	/**
	 * Updates widgets.
	 * \entry	Only from UI thread.
	 */
	void
	update_widgets();

	/**
	 * Recomputes wave.
	 * \entry	Only from UI thread.
	 */
	void
	recompute_wave();

  private:
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

	/**
	 * Deletes object if not 0 and sets pointer to 0.
	 * \entry	Any thread.
	 * \threadsafe when called on distinct arguments.
	 */
	template<class Type>
		void
		drop (Type& pointer)
		{
			delete pointer;
			pointer = 0;
		}

  private:
	Mikuru*						_mikuru;
	Part*						_part;
	Params::Waveform			_params;
	bool						_loading_params;
	Waves						_waves;
	Waves						_modulator_waves;
	WaveComputer*				_wave_computer;
	Shared<DSP::Wave>			_plotters_wave;

	// Ports:
	Core::EventPort*			_port_wave_shape;
	Core::EventPort*			_port_modulator_amplitude;
	Core::EventPort*			_port_modulator_index;
	Core::EventPort*			_port_modulator_shape;

	// Widgets and knobs:
	Haruhi::Knob*				_knob_wave_shape;
	Haruhi::Knob*				_knob_modulator_amplitude;
	Haruhi::Knob*				_knob_modulator_index;
	Haruhi::Knob*				_knob_modulator_shape;
	QWidget*					_panel;
	Haruhi::WavePlot*			_base_wave_plot;
	Haruhi::WavePlot*			_final_wave_plot;
	QComboBox*					_wave_type;
	QComboBox*					_modulator_type;
	QComboBox*					_modulator_wave_type;
	Sliders						_harmonics_sliders;
	Buttons						_harmonics_resets;
	Sliders						_phases_sliders;
	Buttons						_phases_resets;
	QColor						_std_button_bg;
	QColor						_std_button_fg;
	QWidget*					_harmonics_tab;
	QWidget*					_phases_tab;
};

} // namespace MikuruPrivate

#endif

