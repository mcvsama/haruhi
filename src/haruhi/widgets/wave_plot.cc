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
#include <algorithm>
#include <cmath>

// Qt:
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QPolygonF>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/utility.h>
#include <haruhi/utility/mutex.h>

// Local:
#include "wave_plot.h"


namespace Haruhi {

WavePlot::WavePlot (QWidget* parent, const char* name):
	QWidget (parent, name, Qt::WNoAutoErase),
	_last_enabled_state (isEnabled())
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setBackgroundColor (QColor (0xff, 0xff, 0xff));
}


WavePlot::WavePlot (DSP::Wave* wave, QWidget* parent, const char* name):
	WavePlot (parent, name)
{
	assign_wave (wave, false, false);
}


void
WavePlot::assign_wave (DSP::Wave* wave, bool dont_scale_wave, bool dont_scale_grid, bool invert)
{
	Mutex::Lock lock (_samples_mutex);
	_wave = wave;
	_wave_is_immutable = wave && wave->immutable();
	_dont_scale_wave = dont_scale_wave;
	_dont_scale_grid = dont_scale_grid;
	_invert = invert;
}


void
WavePlot::post_plot_shape()
{
	resample_wave();
	QApplication::postEvent (this, new UpdateRequest(), Qt::LowEventPriority);
}


void
WavePlot::plot_shape()
{
	resample_wave();
	update();
}


void
WavePlot::resizeEvent (QResizeEvent*)
{
	if (_prev_size != size())
		plot_shape();
}


void
WavePlot::paintEvent (QPaintEvent* paint_event)
{
	if (_prev_size != size())
		plot_shape();

	if (_to_repaint_buffer || _last_enabled_state != isEnabled())
	{
		int w = width();
		int h = height();

		_double_buffer.resize (w, h);
		QPainter painter (&_double_buffer);
		painter.fillRect (rect(), isEnabled() ? QColor (0xff, 0xff, 0xff) : QColor (0xfa, 0xfa, 0xfa));
		painter.setFont (Resources::small_font());

		// Guards also _denominator:
		_samples_mutex.lock();

		// Use 1.0 denominator if don_scale_grid was set:
		float wave_denominator = _dont_scale_wave ? 1.0f : _denominator;
		float grid_denominator = _dont_scale_grid ? 1.0f : _denominator;

		if (_wave_is_immutable && w > 1 && h > 1 && _samples.size() > 1)
		{
			QColor grid_color = isEnabled() ? QColor (0xd7, 0xd7, 0xd7) : QColor (0xe0, 0xe0, 0xe0);

			// Compute shape:
			int n = _samples.size();
			QPointF point;
			QPolygonF shape_line, shape_polygon;
			for (int x = 0; x < n; ++x)
			{
				point = QPointF ((1.0f / Oversampling) * x, 0.5f * (h - 1) * (1.0f - _samples[x] / wave_denominator));
				shape_line << point;
				shape_polygon << point;
			}
			_samples_mutex.unlock();

			shape_polygon << QPointF (n, shape_polygon[n-1].y());
			shape_polygon << QPointF (n, h / 2);
			shape_polygon << QPointF (-1, h / 2);
			shape_polygon << QPointF (-1, shape_polygon[0].y());

			// Grid:
			painter.setPen (QPen (grid_color, 0.5, Qt::SolidLine));
			painter.drawLine (w / 2, 0, w / 2, h);
			painter.drawLine (0, h / 2, w, h / 2);
			// Half value:
			painter.setPen (QPen (grid_color, 1, Qt::DotLine));
			painter.drawLine (w / 4, 0, w / 4, h);
			painter.drawLine (3 * w / 4, 0, 3 * w / 4, h);
			painter.drawLine (0, h / 2 - (h / 4 / grid_denominator), w, h / 2 - (h / 4 / grid_denominator));
			painter.drawLine (0, h / 2 + (h / 4 / grid_denominator), w, h / 2 + (h / 4 / grid_denominator));
			// 0dB lines if scaled:
			if (grid_denominator > 1.01)
			{
				painter.setPen (QPen (grid_color, 1, Qt::SolidLine));
				painter.drawLine (0, h / 2 - (h / 2 / grid_denominator), w, h / 2 - (h / 2 / grid_denominator));
				painter.drawLine (0, h / 2 + (h / 2 / grid_denominator), w, h / 2 + (h / 2 / grid_denominator));
			}

			painter.translate (0, 0.5f);
			painter.setRenderHint (QPainter::Antialiasing, true);

			// Draw polygon:
			if (_filled_wave)
			{
				painter.setPen (Qt::NoPen);
				painter.setBrush (isEnabled() ? QColor (0xf1, 0xf4, 0xff, 0x7f) : QColor (0xfb, 0xfb, 0xfb, 0x7f));
				painter.drawPolygon (shape_polygon);
			}

			// Draw shape line:
			painter.setPen (QPen (isEnabled() ? QColor (0, 0, 0) : QColor (0xca, 0xca, 0xca), 1.0, Qt::SolidLine));
			painter.setBrush (Qt::NoBrush);
			painter.drawPolyline (shape_line);
		}
		else
			_samples_mutex.unlock();

		if (_wave_is_immutable)
		{
			// Phase marker:
			if (_phase_enabled)
			{
				QColor color = isEnabled() ? QColor (0x00, 0x00, 0xdd) : QColor (0xc0, 0xc0, 0xc0);
				painter.setRenderHint (QPainter::Antialiasing, false);
				painter.setPen (QPen (color, 1, Qt::DotLine));
				painter.setBrush (Qt::NoBrush);
				int xpos = std::min (static_cast<int> (_phase_position * w), w - 1);
				painter.drawLine (xpos, 0, xpos, h);
			}
		}
		// Not immutable wave?
		else
		{
			painter.setRenderHint (QPainter::Antialiasing, true);
			painter.setPen (QPen (isEnabled() ? QColor (0xaa, 0xaa, 0xaa) : QColor (0xee, 0xee, 0xee), 0.5, Qt::SolidLine));
			painter.setBrush (Qt::NoBrush);
			painter.drawLine (0, 0, w, h);
			painter.drawLine (0, h, w, 0);
		}
	}
	QPainter (this).drawPixmap (paint_event->rect().topLeft(), _double_buffer, paint_event->rect());
	_to_repaint_buffer = false;
	_last_enabled_state = isEnabled();

	_prev_size = size();
}


void
WavePlot::resample_wave()
{
	Mutex::Lock lock (_samples_mutex);
	if (_wave)
	{
		// Don't bother if wave is not immutable.
		if (_wave->immutable())
		{
			_samples.resize (std::max (1, Oversampling * width()));
			_denominator = 1.0;
			int n = _samples.size();
			float inverter = _invert ? -1.0f : 1.0f;
			int const m = _closed_ring ? n - 1 : n;
			for (int x = 0; x < m; ++x)
			{
				_samples[x] = inverter * (*_wave)(1.0f * x / n, 0);
				_denominator = std::max (_denominator, std::abs (_samples[x]));
			}
			// Since wave(0.0) should be the same as wave(1.0), we'll use wave(0.0)
			// for the last sample, but only if this feature is enabled (closed ring):
			if (_closed_ring)
				_samples[n - 1] = inverter * (*_wave)(0.0f, 0);
		}
		_to_repaint_buffer = true;
	}
}


void
WavePlot::customEvent (QEvent* event)
{
	if (dynamic_cast<UpdateRequest*> (event))
		update();
}

} // namespace Haruhi

