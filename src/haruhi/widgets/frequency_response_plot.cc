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

// Standard:
#include <cstddef>
#include <limits>

// Qt:
#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QPolygonF>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>
#include <haruhi/application/services.h>
#include <haruhi/dsp/utility.h>

// Local:
#include "frequency_response_plot.h"


namespace Haruhi {

using namespace ScreenLiterals;


FrequencyResponsePlot::FrequencyResponsePlot (QWidget* parent):
	QWidget (parent),
	_last_enabled_state (isEnabled())
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QPalette p = palette();
	p.setColor (QPalette::Window, Qt::white);
	setPalette (p);
	setAutoFillBackground (false);
}


FrequencyResponsePlot::FrequencyResponsePlot (DSP::PlotableImpulseResponse* impulse_response, QWidget* parent):
	FrequencyResponsePlot (parent)
{
	assign_impulse_response (impulse_response);
}


void
FrequencyResponsePlot::set_num_stages (float num_stages)
{
	_num_stages = num_stages;
	repaint_grid();
	replot();
}


void
FrequencyResponsePlot::assign_impulse_response (DSP::PlotableImpulseResponse* impulse_response)
{
	_impulse_response = impulse_response;
	replot();
}


void
FrequencyResponsePlot::replot (bool force)
{
	if (force)
	{
		if (_impulse_response)
		{
			_values.resize (width());
			int n = _values.size();
			int s = 2 * MaxFreq;
			for (int x = 0; x < n; ++x)
			{
				float k = std::exp (1.0f * x / n * std::log (static_cast<float> (MaxFreq))) + MinFreq;
				float r = _impulse_response->response (k / s + 1.0 / (2 * MaxFreq));
				_values[x] = r >= 0.0
					? _num_stages * 10.0 * std::log10 (r)
					: -std::numeric_limits<float>::infinity();
			}
			_to_repaint_buffer = true;
			_to_replot = false;
			_prev_size = size();
		}
	}
	else
	{
		_to_replot = true;
		update();
	}
}


void
FrequencyResponsePlot::post_replot()
{
	QApplication::postEvent (this, new Replot());
}


void
FrequencyResponsePlot::resizeEvent (QResizeEvent*)
{
	if (_prev_size != size())
	{
		repaint_grid();
		replot();
	}
}


void
FrequencyResponsePlot::paintEvent (QPaintEvent* paint_event)
{
	if (_to_replot)
		replot (true);

	if (_last_enabled_state != isEnabled())
		repaint_grid();

	if (_to_repaint_buffer || _last_enabled_state != isEnabled())
	{
		int h = height();

		const float lower_db = -70;
		const float upper_db = +30;

		_double_buffer = QPixmap (size());
		QPainter painter (&_double_buffer);

		// Draw grid:
		painter.drawPixmap (QPoint (0, 0), _grid_buffer);

		// Compute response:
		int n = _values.size();
		QPointF point;
		QPolygonF response_line, response_polygon;
		for (int x = 0; x < n; ++x)
		{
			point = QPointF (x, ifnan (h - log_meter (_values[x], lower_db, upper_db) * h, 0.0f));
			response_polygon << point;
			response_line << point;
		}
		response_polygon << QPointF (n + 2, response_polygon[n-1].y());
		response_polygon << QPointF (n + 2, h + 2);
		response_polygon << QPointF (-2, h + 2);
		response_polygon << QPointF (-2, response_polygon[0].y());

		// Paint polygon:
		painter.translate (0, 0.5f);
		painter.setRenderHint (QPainter::Antialiasing, true);
		painter.setPen (Qt::NoPen);
		painter.setBrush (isEnabled() ? QColor (0xe0, 0xe7, 0xff, 0x7f) : QColor (0xeb, 0xeb, 0xeb, 0x7f));
		painter.drawPolygon (response_polygon);

		// Draw response line:
		painter.setPen (QPen (isEnabled() ? QColor (0, 0, 0) : QColor (0xca, 0xca, 0xca), 0.15_screen_mm, Qt::SolidLine));
		painter.setBrush (Qt::NoBrush);
		painter.drawPolyline (response_line);
	}
	QPainter (this).drawPixmap (paint_event->rect().topLeft(), _double_buffer, paint_event->rect());
	_to_repaint_buffer = false;
	_last_enabled_state = isEnabled();
}


void
FrequencyResponsePlot::customEvent (QEvent* e)
{
	if (dynamic_cast<Replot*> (e))
		replot();
}


void
FrequencyResponsePlot::repaint_grid()
{
	int w = width();
	int h = height();

	const float lower_db = -70;
	const float upper_db = +30;

	_grid_buffer = QPixmap (size());

	QPainter painter (&_grid_buffer);
	painter.fillRect (rect(), isEnabled() ? QColor (0xff, 0xff, 0xff) : QColor (0xfa, 0xfa, 0xfa, 0xff));
	painter.setFont (Resources::small_font());

	QColor grid_color = isEnabled() ? QColor (0xcc, 0xcc, 0xcc) : QColor (0xe0, 0xe0, 0xe0);

	// Scale markers [dB]:
	painter.setPen (QPen (grid_color, 0.15_screen_mm, Qt::DotLine));
	for (float db: { -60, -50, -40, -30, -20, -10, 0, +10, +20 })
	{
		float pos = log_meter (db, lower_db, upper_db) * h;
		painter.drawLine (0, h - pos, w, h - pos);
		float const scale = 1.0f;
		if (db >= -40)
			painter.drawText (0.8_screen_mm, h - pos - 0.3_screen_mm, QString::number (std::abs (scale * db)) + ((db == 0) ? " dB" : ""));
	}

	// 0dB line:
	painter.setPen (QPen (grid_color, 0.15_screen_mm, Qt::SolidLine));
	{
		float pos = log_meter (0, lower_db, upper_db) * h;
		painter.drawLine (0, h - pos, w, h - pos);
	}

	// Frequency markers:
	painter.setPen (QPen (grid_color, 0.15_screen_mm, Qt::SolidLine));
	bool drawn_first = false;
	for (float f: { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000 })
	{
		if (f >= MinFreq && f <= MaxFreq)
		{
			float pos = w * log (f - MinFreq) / log (MaxFreq);
			painter.drawLine (pos, 0, pos, h);
			// Draw first frequency, 100Hz, 1k and 10k:
			if (!drawn_first)
			{
				painter.drawText (pos + 2, 2_screen_mm, QString::number (f));
				drawn_first = true;
			}
			else if (f == 100)
				painter.drawText (pos + 2, 2_screen_mm, QString::number (f));
			else if (f == 1000 || f == 10000)
				painter.drawText (pos + 2, 2_screen_mm, QString::number (f / 1000) + "k");
		}
	}
}

} // namespace Haruhi

