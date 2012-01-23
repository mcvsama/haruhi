/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__WIDGETS__WAVE_PLOT_H__INCLUDED
#define HARUHI__WIDGETS__WAVE_PLOT_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtCore/QEvent>
#include <QtGui/QWidget>
#include <QtGui/QPixmap>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/graph.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/utility/mutex.h>


namespace Haruhi {

class WavePlot: public QWidget
{
	enum {
		Oversampling = 3,
	};

	class UpdateRequest: public QEvent
	{
	  public:
		UpdateRequest() noexcept:
			QEvent (QEvent::User)
		{ }
	};

  public:
	/**
	 * Creates plot without assigned wave.
	 */
	WavePlot (QWidget* parent, const char* name = 0);

	/**
	 * Creates plot and assigns wave to it.
	 * \param	wave is wave object.
	 */
	WavePlot (DSP::Wave* wave, QWidget* parent, const char* name = 0);

	/**
	 * Dtor
	 * Does NOT delete wave object.
	 */
	virtual ~WavePlot();

	/**
	 * Assigns Wave object to this plot.
	 * WavePlot does not take ownership of Wave object.
	 *
	 * \param	wave is the wave to use. 0 deassigns wave.
	 * \param	dont_scale_grid tells to draw always the same grid
	 * 			regardless of min/max values of wave.
	 * \param	invert tells whether to invert wave plot on screen vertically.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	void
	assign_wave (DSP::Wave* wave, bool dont_scale_wave = false, bool dont_scale_grid = false, bool invert = false);

	/**
	 * \returns	currently assigned Wave object (0 if none).
	 * \entry	Any thread.
	 */
	DSP::Wave*
	wave() const;

	/**
	 * Replots the wave.
	 * \entry	Only from UI thread.
	 */
	void
	plot_shape();

	/**
	 * Replots wave and requests update() by sending QEvent to itself in main thread.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	void
	post_plot_shape();

	/**
	 * Enables/disables phase marker.
	 * \param	enable: Enable phase marker?
	 */
	void
	set_phase_marker_enabled (bool enable);

	/**
	 * Sets position of phase marker.
	 * \param	position: [0..1].
	 */
	void
	set_phase_marker_position (float position);

  protected:
	void
	resizeEvent (QResizeEvent*);

	void
	paintEvent (QPaintEvent*);

  private:
	/**
	 * Resamples wave, but does not update widget.
	 * It's called by plot_shape() and indirectly by post_plot_shape().
	 */
	void
	resample_wave();

	void
	configure_widget();

	void
	customEvent (QEvent* event);

  private:
	QPixmap				_double_buffer;
	bool				_to_repaint_buffer;
	bool				_last_enabled_state;
	DSP::Wave*			_wave;
	bool				_wave_is_immutable;
	std::vector<float>	_samples;
	Mutex				_samples_mutex;
	QSize				_prev_size;
	float				_denominator;
	bool				_dont_scale_wave;
	bool				_dont_scale_grid;
	bool				_invert;
	bool				_phase_enabled;
	float				_phase_position;
};


inline DSP::Wave*
WavePlot::wave() const
{
	return _wave;
}


inline void
WavePlot::set_phase_marker_enabled (bool enable)
{
	_phase_enabled = enable;
}


inline void
WavePlot::set_phase_marker_position (float position)
{
	_phase_position = position;
}

} // namespace Haruhi

#endif

