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

#ifndef HARUHI__WIDGETS__FREQUENCY_RESPONSE_PLOT_H__INCLUDED
#define HARUHI__WIDGETS__FREQUENCY_RESPONSE_PLOT_H__INCLUDED

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
#include <haruhi/dsp/impulse_response.h>


namespace Haruhi {

class FrequencyResponsePlot: public QWidget
{
	Q_OBJECT

	enum {
		MinFreq = 32,
		MaxFreq = 24000,
	};

	/**
	 * Used to request replot from other-than-UI thread.
	 */
	class Replot: public QEvent
	{
	  public:
		Replot() noexcept:
			QEvent (QEvent::User)
		{ }
	};

  public:
	FrequencyResponsePlot (QWidget* parent, const char* name = 0);

	FrequencyResponsePlot (DSP::PlotableImpulseResponse* impulse_response, QWidget* parent, const char* name = 0);

	virtual ~FrequencyResponsePlot() = default;

	/**
	 * Sets number of stages the same filter will be applied.
	 * Causes plot to be rendered differently.
	 */
	void
	set_num_stages (float num_stages);

	/**
	 * Assigns PlotableImpulseResponse to this plot.
	 * FrequencyResponsePlot does not take ownership of PlotableImpulseResponse object.
	 * Pass 0 to deassign.
	 */
	void
	assign_impulse_response (DSP::PlotableImpulseResponse* impulse_response);

	/**
	 * Returns currently assigned PlotableImpulseResponse (or 0 if none assigned).
	 */
	DSP::PlotableImpulseResponse*
	impulse_response() const noexcept;

	/**
	 * Request replot on nearest paint event.
	 * \param	force forces replot to be done now.
	 * \thread	Only UI thread. Use post_replot() from other threads.
	 */
	void
	replot (bool force = false);

	/**
	 * Request replot on nearest paint event.
	 * \threadsafe
	 */
	void
	post_replot();

  protected:
	void
	resizeEvent (QResizeEvent*) override;

	void
	paintEvent (QPaintEvent*) override;

	void
	customEvent (QEvent*) override;

  private:
	/**
	 * Paints grid using DotLine style on buffer,
	 * which is workaround for extremely slow Qt4 in this task.
	 */
	void
	repaint_grid();

  private:
	Graph*							_graph					= nullptr;
	float							_num_stages				= 1.0f;
	QPixmap							_double_buffer;
	QPixmap							_grid_buffer;
	bool							_to_repaint_buffer		= false;
	bool							_to_replot				= false;
	bool							_last_enabled_state;
	DSP::PlotableImpulseResponse*	_impulse_response		= nullptr;
	std::vector<float>				_values;
	QSize							_prev_size;
};


inline DSP::PlotableImpulseResponse*
FrequencyResponsePlot::impulse_response() const noexcept
{
	return _impulse_response;
}

} // namespace Haruhi

#endif

