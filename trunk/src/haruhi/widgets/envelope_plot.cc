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

// Standard:
#include <cstddef>
#include <algorithm>
#include <cmath>
#include <stdint.h>

// Qt:
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QPolygonF>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/dsp/utility.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "envelope_plot.h"


EnvelopePlot::EnvelopePlot (QWidget* parent, const char* name):
	QWidget (parent, name, Qt::WNoAutoErase),
	_sample_rate (1),
	_to_repaint_buffer (false),
	_last_enabled_state (isEnabled()),
	_envelope (0)
{
	configure_widget();
}


EnvelopePlot::EnvelopePlot (DSP::Envelope* envelope, QWidget* parent, const char* name):
	QWidget (parent, name, Qt::WNoAutoErase),
	_to_repaint_buffer (false),
	_last_enabled_state (isEnabled()),
	_envelope (0)
{
	configure_widget();
	assign_envelope (envelope);
}


EnvelopePlot::~EnvelopePlot()
{
}


void
EnvelopePlot::assign_envelope (DSP::Envelope* envelope)
{
	atomic (_envelope) = envelope;
}


void
EnvelopePlot::post_plot_shape()
{
	_to_repaint_buffer = true;
	QApplication::postEvent (this, new UpdateRequest(), Qt::LowEventPriority);
}


void
EnvelopePlot::plot_shape()
{
	_to_repaint_buffer = true;
	update();
}


void
EnvelopePlot::resizeEvent (QResizeEvent*)
{
	if (_prev_size != size())
		plot_shape();
}


void
EnvelopePlot::paintEvent (QPaintEvent* paint_event)
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
		painter.setFont (Config::small_font);

		if (w > 1 && h > 1)
		{
			QColor grid_color = isEnabled() ? QColor (0xd7, 0xd7, 0xd7) : QColor (0xe0, 0xe0, 0xe0);
			DSP::Envelope::Points& points = _envelope->points();

			int sum_samples = 0;
			int sustain_sample = 0;
			float sustain_value = 0.0f;
			for (DSP::Envelope::Points::iterator p = points.begin(); p != points.end(); ++p)
			{
				if (p->sustain)
				{
					sustain_sample = sum_samples;
					sustain_value = p->value;
				}
				sum_samples += p->samples;
			}

			// Compute shape:
			QPointF point;
			QPolygonF shape_line, shape_polygon;
			int s = 0;
			if (sum_samples > 0)
			{
				for (DSP::Envelope::Points::iterator p = points.begin(); p != points.end(); ++p)
				{
					point = QPointF (1.0f * s * w / sum_samples, h - 1 - p->value * (h - 1));
					shape_line << point;
					shape_polygon << point;
					s += p->samples;
				}
			}
			else
			{
				point = QPointF (0, h - 1 - sustain_value * (h - 1));
				shape_line << point;
				shape_polygon << point;
				point = QPointF (w, h - 1 - sustain_value * (h - 1));
				shape_line << point;
				shape_polygon << point;
			}
			shape_polygon << QPointF (w, h);
			shape_polygon << QPointF (-1, h);

			// Grid:
			painter.setPen (QPen (grid_color, 0.5, Qt::SolidLine));
			// 1ms, 10ms, 100ms, 1s, 10s lines:
			float ks[] = { 0.001, 0.01, 0.1, 1, 10 };
			int ks_size = sizeof ks / sizeof *ks;
			for (float* k = ks; k != ks + ks_size; ++k)
			{
				if (sum_samples < 15.0 * *k * _sample_rate)
				{
					for (int64_t i = 1; i <= sum_samples / *k / _sample_rate; ++i)
					{
						int64_t x = i * _sample_rate * w / (sum_samples / *k);
						painter.drawLine (x, 0, x, h);
						painter.translate (+x + 3, +2);
						painter.rotate (+90);
						painter.drawText (0, 0, QString::number (i * *k) + " s");
						painter.rotate (-90);
						painter.translate (-x - 3, -2);
					}
					break;
				}
			}

			// Draw polygon:
			painter.translate (0, 0.5f);
			painter.setRenderHint (QPainter::Antialiasing, true);
			painter.setPen (Qt::NoPen);
			painter.setBrush (isEnabled() ? QColor (0xf1, 0xf4, 0xff, 0x7f) : QColor (0xfb, 0xfb, 0xfb, 0x7f));
			painter.drawPolygon (shape_polygon);

			// Draw shape line:
			painter.setPen (QPen (isEnabled() ? QColor (0, 0, 0) : QColor (0xca, 0xca, 0xca), 1.0, Qt::SolidLine));
			painter.setBrush (Qt::NoBrush);
			painter.drawPolyline (shape_line);

			// Sustain point marker:
			painter.setRenderHint (QPainter::Antialiasing, false);
			painter.setPen (QPen (QColor (0xdd, 0x00, 0x00), 1, Qt::DotLine));
			painter.setBrush (Qt::NoBrush);
			int xpos = std::min (static_cast<int> (1.0f * sustain_sample / sum_samples * w), w - 1);
			painter.drawLine (xpos, h - sustain_value * h, xpos, h);
		}
	}
	QPainter (this).drawPixmap (paint_event->rect().topLeft(), _double_buffer, paint_event->rect());
	_to_repaint_buffer = false;
	_last_enabled_state = isEnabled();

	_prev_size = size();
}


void
EnvelopePlot::configure_widget()
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setBackgroundColor (QColor (0xff, 0xff, 0xff));
}


void
EnvelopePlot::customEvent (QEvent* event)
{
	if (dynamic_cast<UpdateRequest*> (event))
		update();
}

