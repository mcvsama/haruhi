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

#ifndef HARUHI__WIDGET__ENVELOPE_PLOT_H__INCLUDED
#define HARUHI__WIDGET__ENVELOPE_PLOT_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtCore/QEvent>
#include <QtGui/QWidget>
#include <QtGui/QPixmap>

// Haruhi:
#include <haruhi/core/graph.h>
#include <haruhi/dsp/envelope.h>


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
	EnvelopePlot (QWidget* parent, const char* name = 0);

	/**
	 * Creates plot and assigns envelope to it.
	 * \param	envelope is envelope object.
	 */
	EnvelopePlot (DSP::Envelope* envelope, QWidget* parent, const char* name = 0);

	/**
	 * Dtor
	 * Does NOT delete envelope object.
	 */
	virtual ~EnvelopePlot();

	/**
	 * Sets sample rate for Plot to correctly display
	 * time bars.
	 */
	void
	set_sample_rate (unsigned int sample_rate) { _sample_rate = sample_rate; }

	/**
	 * Assigns Envelope object to this plot.
	 * EnvelopePlot does not take ownership of Envelope object.
	 *
	 * \param	envelope is the envelope to use. 0 deassigns envelope.
	 * \entry	Any thread.
	 * \threadsafe
	 */
	void
	assign_envelope (DSP::Envelope* envelope);

	/**
	 * \returns	currently assigned Envelope object (0 if none).
	 * \entry	Any thread.
	 * \threadsafe
	 */
	DSP::Envelope*
	envelope() const { return atomic (_envelope); }

	/**
	 * Sets editable mode of the plot.
	 * \param	max_segment_time is maximum time in seconds for one env. segment.
	 * \entry	Qt thread only.
	 */
	void
	set_editable (bool editable, float max_segment_time = 0.0f) { _editable = editable; _max_segment_time = max_segment_time; }

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
	envelope_updated();

  protected:
	void
	resizeEvent (QResizeEvent*);

	void
	paintEvent (QPaintEvent*);

	void
	enterEvent (QEvent*);

	void
	leaveEvent (QEvent*);

	void
	mouseMoveEvent (QMouseEvent*);

	void
	mousePressEvent (QMouseEvent*);

	void
	mouseReleaseEvent (QMouseEvent*);

  private:
	void
	configure_widget();

	void
	customEvent (QEvent* event);

  private:
	unsigned int		_sample_rate;
	QPixmap				_double_buffer;
	bool				_force_repaint;
	bool				_last_enabled_state;
	DSP::Envelope*		_envelope;
	QSize				_prev_size;
	bool				_editable;
	float				_max_segment_time; // in seconds
	// True when mouse is over the plot:
	bool				_hovered;
	// Mouse position over the plot:
	QPoint				_mouse_pos;
	// Index of dragged point or -1 if none:
	int					_current_point_index;
	// Samples number for current point in envelope:
	int					_current_point_samples;
	float				_current_point_value;
	bool				_dragging;
	QPoint				_drag_start_pos;
};

#endif

