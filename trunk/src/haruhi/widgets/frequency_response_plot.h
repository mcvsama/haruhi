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

#ifndef HARUHI__WIDGET__FREQUENCY_RESPONSE_PLOT_H__INCLUDED
#define HARUHI__WIDGET__FREQUENCY_RESPONSE_PLOT_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QPixmap>

// Haruhi:
#include <haruhi/dsp/impulse_response.h>


class FrequencyResponsePlot: public QWidget
{
	Q_OBJECT

	enum {
		MinFreq = 32,
		MaxFreq = 24000,
	};

  public:
	FrequencyResponsePlot (QWidget* parent, const char* name = 0);

	FrequencyResponsePlot (DSP::ImpulseResponse* impulse_response, QWidget* parent, const char* name = 0);

	virtual ~FrequencyResponsePlot();

	void
	set_double_scale (bool set);

	/**
	 * Assigns ImpulseResponse to this plot.
	 * FrequencyResponsePlot does not take ownership of ImpulseResponse object.
	 * Pass 0 to deassign.
	 */
	void
	assign_impulse_response (DSP::ImpulseResponse* impulse_response);

	/**
	 * Returns currently assigned ImpulseResponse (or 0 if none assigned).
	 */
	DSP::ImpulseResponse*
	impulse_response() const { return _impulse_response; }

	/**
	 * Request replot on nearest paint event.
	 * \param	force forces replot to be done now.
	 */
	void
	replot (bool force = false);

  protected:
	void
	resizeEvent (QResizeEvent*);

	void
	paintEvent (QPaintEvent*);

  private:
	void
	configure_widget();

	/**
	 * Paints grid using DotLine style on buffer,
	 * which is workaround for extremely slow Qt4 in this task.
	 */
	void
	repaint_grid();

  private:
	Core::Graph*			_graph;
	bool					_double_scale;
	QPixmap					_double_buffer;
	QPixmap					_grid_buffer;
	bool					_to_repaint_buffer;
	bool					_to_replot;
	bool					_last_enabled_state;
	DSP::ImpulseResponse*	_impulse_response;
	std::vector<float>		_values;
	QSize					_prev_size;
};

#endif

