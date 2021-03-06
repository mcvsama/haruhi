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

#ifndef HARUHI__WIDGETS__ENVELOPE_PLOT_H__INCLUDED
#define HARUHI__WIDGETS__ENVELOPE_PLOT_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QEvent>
#include <QWidget>
#include <QPixmap>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/graph.h>
#include <haruhi/dsp/envelope.h>


namespace Haruhi {

class EnvelopePlot: public QWidget
{
	Q_OBJECT

	class UpdateRequest: public QEvent
	{
	  public:
		UpdateRequest():
			QEvent (QEvent::User)
		{ }
	};

  public:
	/**
	 * Creates plot without assigned envelope.
	 */
	EnvelopePlot (QWidget* parent);

	/**
	 * Creates plot and assigns envelope to it.
	 * \param	envelope is envelope object.
	 */
	EnvelopePlot (DSP::Envelope* envelope, QWidget* parent);

	/**
	 * Sets sample rate for Plot to correctly display
	 * time bars.
	 */
	void
	set_sample_rate (Frequency sample_rate) noexcept;

	/**
	 * Assigns Envelope object to this plot.
	 * EnvelopePlot does not take ownership of Envelope object.
	 *
	 * \param	envelope is the envelope to use. nullptr deassigns envelope.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	void
	assign_envelope (DSP::Envelope* envelope);

	/**
	 * \returns	currently assigned Envelope object (0 if none).
	 * \entry	Any thread.
	 */
	DSP::Envelope*
	envelope() const noexcept;

	/**
	 * Sets editable mode of the plot.
	 * \param	max_segment_time is maximum time in seconds for one env. segment.
	 * \entry	Qt thread only.
	 */
	void
	set_editable (bool editable, Time max_segment_time) noexcept;

	/**
	 * \returns	Currently edited point or -1 if no point is active.
	 * \entry	Any thread.
	 */
	int
	active_point() const noexcept;

	/**
	 * Sets current edited point.
	 * Pass -1 to disable active point.
	 * \entry	Qt thread only.
	 */
	void
	set_active_point (int index);

	/**
	 * Replots the envelope.
	 * \entry	Qt thread only.
	 */
	void
	plot_shape();

	/**
	 * Replots envelope and requests update() by sending QEvent to itself in main thread.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	void
	post_plot_shape();

  signals:
	void
	active_point_changed (unsigned int index);

	void
	envelope_updated();

	void
	active_point_changed();

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

	void
	enterEvent (QEvent*) override;

	void
	leaveEvent (QEvent*) override;

	void
	mouseMoveEvent (QMouseEvent*) override;

	void
	mousePressEvent (QMouseEvent*) override;

	void
	mouseReleaseEvent (QMouseEvent*) override;

  private:
	void
	customEvent (QEvent* event) override;

  private:
	Frequency				_sample_rate				= 1_Hz;
	QPixmap					_double_buffer;
	bool					_force_repaint				= false;
	bool					_last_enabled_state;
	Atomic<DSP::Envelope*>	_envelope;
	QSize					_prev_size;
	bool					_editable					= false;
	Time					_max_segment_time			= 64_s;
	// True when mouse is over the plot:
	bool					_hovered					= false;
	// Mouse position over the plot:
	QPoint					_mouse_pos;
	// Index of active/dragged point or -1 if none:
	int						_active_point_index			= -1;
	int						_hovered_point_index		= -1;
	// Samples number for active point in envelope:
	int						_active_point_samples		= 0;
	float					_active_point_value			= 0.0f;
	bool					_dragging					= false;
	QPoint					_drag_start_pos;
};


inline void
EnvelopePlot::set_sample_rate (Frequency sample_rate) noexcept
{
	_sample_rate = sample_rate;
}


inline DSP::Envelope*
EnvelopePlot::envelope() const noexcept
{
	return _envelope.load();
}


inline void
EnvelopePlot::set_editable (bool editable, Time max_segment_time) noexcept
{
	_editable = editable;
	_max_segment_time = max_segment_time;
}


inline int
EnvelopePlot::active_point() const noexcept
{
	return _active_point_index;
}

} // namespace Haruhi

#endif

