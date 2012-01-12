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
#include <haruhi/config/all.h>
#include <haruhi/dsp/utility.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "envelope_plot.h"


namespace Haruhi {

EnvelopePlot::EnvelopePlot (QWidget* parent, const char* name):
	QWidget (parent, name, Qt::WNoAutoErase),
	_sample_rate (1),
	_force_repaint (false),
	_last_enabled_state (isEnabled()),
	_envelope (0),
	_editable (false),
	_hovered (false),
	_active_point_index (-1),
	_hovered_point_index (-1),
	_dragging (false)
{
	configure_widget();
}


EnvelopePlot::EnvelopePlot (DSP::Envelope* envelope, QWidget* parent, const char* name):
	QWidget (parent, name, Qt::WNoAutoErase),
	_force_repaint (false),
	_last_enabled_state (isEnabled()),
	_envelope (0),
	_editable (false),
	_hovered (false),
	_active_point_index (-1),
	_hovered_point_index (-1),
	_dragging (false)
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
EnvelopePlot::set_active_point (int index)
{
	if (index == -1)
		_active_point_index = index;
	else if (index >= 0 && index < static_cast<int> (_envelope->points().size()))
	{
		_active_point_index = index;
		// We'll be altering length of previous point and value of the current:
		_active_point_samples = index > 0 ? _envelope->points()[index - 1].samples : 0;
		_active_point_value = _envelope->points()[index].value;

		_force_repaint = true;
		update();

		emit active_point_changed (index);
	}
}


void
EnvelopePlot::plot_shape()
{
	_force_repaint = true;
	update();
}


void
EnvelopePlot::post_plot_shape()
{
	_force_repaint = true;
	QApplication::postEvent (this, new UpdateRequest(), Qt::LowEventPriority);
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

	if (_force_repaint || _last_enabled_state != isEnabled())
	{
		int w = width();
		int h = height();

		_double_buffer.resize (w, h);
		QPainter painter (&_double_buffer);
		painter.fillRect (rect(), isEnabled() ? QColor (0xff, 0xff, 0xff) : QColor (0xfa, 0xfa, 0xfa));
		painter.setFont (Resources::small_font());
		// Collect envelope points on canvas:
		std::vector<QPointF> envelope_points;

		if (w > 1 && h > 1)
		{
			QColor grid_color = isEnabled() ? QColor (0xd7, 0xd7, 0xd7) : QColor (0xe0, 0xe0, 0xe0);
			DSP::Envelope::Points& points = _envelope->points();

			int sum_samples = 0;
			int sustain_sample = 0;
			float sustain_value = 0.0f;
			unsigned int i = 0;
			for (DSP::Envelope::Point& p: points)
			{
				if (_envelope->sustain_point() == i)
				{
					sustain_sample = sum_samples;
					sustain_value = p.value;
				}
				sum_samples += p.samples;
				++i;
			}

			// Compute shape:
			QPointF point;
			QPolygonF shape_line, shape_polygon;
			int s = 0;
			if (sum_samples > 0)
			{
				for (DSP::Envelope::Point& p: points)
				{
					point = QPointF (1.0f * s * w / sum_samples, h - 1 - p.value * (h - 1));
					envelope_points.push_back (point);
					shape_line << point;
					shape_polygon << point;
					s += p.samples;
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

			// Draw rectangles around envelope points:
			_hovered_point_index = -1;
			if (_editable && !envelope_points.empty())
			{
				QLineF active_line;
				QLineF hovered_line;
				std::vector<QRect> normal_rects;
				QRect active_rect;
				QRect hovered_rect;

				// Go backwards so if two rects are overlapping, the latter has priority to become hovered:
				typedef std::vector<QPointF>::difference_type DT;
				for (auto i = static_cast<DT> (envelope_points.size()) - 1; i >= 0; --i)
				{
					QRect rect = QRect (envelope_points[i].x() - 4, envelope_points[i].y() - 4, 8, 8);
					QLineF line;
					if (i > 0)
						line = QLineF (envelope_points[i-1], envelope_points[i]);
					// Point [i] hovered?
					if ((_dragging && _hovered_point_index == i) || (!_dragging && _hovered && rect.contains (_mouse_pos) && _hovered_point_index == -1))
					{
						_hovered_point_index = i;
						hovered_rect = rect;
						if (i > 0)
							hovered_line = line;
					}
					// Point [i] active?
					if (_active_point_index == i)
					{
						active_rect = rect;
						if (i > 0)
							active_line = line;
					}
					// Not hovered and not active?
					if (i != _hovered_point_index && i != _active_point_index)
						normal_rects.push_back (rect);
				}

				painter.setRenderHint (QPainter::Antialiasing, true);
				// Active line:
				if (_active_point_index != -1 && _active_point_index != _hovered_point_index)
				{
					painter.setPen (QPen (QColor (0x00, 0x00, 0xff), 1.5, Qt::SolidLine));
					painter.drawLine (active_line);
				}
				// Hovered line:
				if (_hovered_point_index != -1)
				{
					painter.setPen (QPen (QColor (0xff, 0x00, 0x00), 1.5, Qt::SolidLine));
					painter.drawLine (hovered_line);
				}
				// Normal points:
				if (_hovered)
				{
					painter.setRenderHint (QPainter::Antialiasing, false);
					painter.setPen (QPen (QColor (0x00, 0x00, 0x00), 1, Qt::SolidLine));
					for (QRect& rect: normal_rects)
						painter.drawRect (rect);
					// Active point:
					painter.setRenderHint (QPainter::Antialiasing, false);
					if (_active_point_index != -1 && _active_point_index != _hovered_point_index)
					{
						painter.setPen (QPen (QColor (0x00, 0x00, 0xbb), 1, Qt::SolidLine));
						painter.fillRect (active_rect.adjusted (0, 0, 1, 1), QColor (0x00, 0x00, 0xff));
						painter.drawRect (active_rect);
					}
				}
				else
				{
					painter.setRenderHint (QPainter::Antialiasing, true);
					painter.setPen (QPen (QColor (0x00, 0x00, 0x00), 1, Qt::SolidLine));
					painter.setBrush (QBrush (QColor (0xca, 0xca, 0xca)));
					for (QRect& rect: normal_rects)
						painter.drawEllipse (rect.adjusted (2, 2, -1, -1));
					// Active point:
					if (_active_point_index != -1 && _active_point_index != _hovered_point_index)
					{
						painter.setPen (QPen (QColor (0x00, 0x00, 0xbb), 1, Qt::SolidLine));
						painter.setBrush (QBrush (QColor (0x00, 0x00, 0xff)));
						painter.drawEllipse (active_rect.adjusted (2, 2, -1, -1));
					}
				}
				// Hovered point:
				if (_hovered_point_index != -1)
				{
					painter.setPen (QPen (QColor (0xcc, 0x00, 0x00), 1, Qt::SolidLine));
					painter.setBrush (Qt::NoBrush);
					painter.fillRect (hovered_rect.adjusted (0, 0, 1, 1), QColor (0xff, 0x00, 0x00));
					painter.drawRect (hovered_rect);
				}
			}
		}
	}
	QPainter (this).drawPixmap (paint_event->rect().topLeft(), _double_buffer, paint_event->rect());
	_force_repaint = false;
	_last_enabled_state = isEnabled();

	_prev_size = size();
}


void
EnvelopePlot::enterEvent (QEvent* event)
{
	event->accept();
	_hovered = true;
	if (_editable)
	{
		_force_repaint = true;
		update();
	}
}


void
EnvelopePlot::leaveEvent (QEvent* event)
{
	event->accept();
	_hovered = false;
	if (_editable)
	{
		_force_repaint = true;
		update();
	}
}


void
EnvelopePlot::mouseMoveEvent (QMouseEvent* event)
{
	event->accept();
	_mouse_pos = event->pos() - QPoint (2, 2);
	if (_editable)
	{
		if (_dragging && _active_point_index != -1)
		{
			const int samples_per_pixel = _sample_rate / 240;
			const int max_samples = _max_segment_time * _sample_rate;
			const int samples_diff = samples_per_pixel * (_drag_start_pos - _mouse_pos).x();
			const unsigned int new_samples = std::max (0, std::min (max_samples, _active_point_samples - samples_diff));
			const float new_value = std::max (0.0f, std::min (1.0f, _active_point_value + 1.0f * (_drag_start_pos - _mouse_pos).y() / height()));
			// Don't allow moving first point horizontally:
			if (_active_point_index > 0)
				_envelope->points()[_active_point_index - 1].samples = new_samples;
			_envelope->points()[_active_point_index].value = new_value;
			// Emit signal:
			emit envelope_updated();
		}
		_force_repaint = true;
		update();
	}
}


void
EnvelopePlot::mousePressEvent (QMouseEvent* event)
{
	if (_editable && event->button() == Qt::LeftButton && _hovered_point_index != -1)
	{
		event->accept();
		_dragging = true;
		_drag_start_pos = event->pos() - QPoint (2, 2);
		set_active_point (_hovered_point_index);
		emit active_point_changed();
	}
}


void
EnvelopePlot::mouseReleaseEvent (QMouseEvent*)
{
	_dragging = false;
	_force_repaint = true;
	update();
}


void
EnvelopePlot::configure_widget()
{
	setMouseTracking (true);
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setBackgroundColor (QColor (0xff, 0xff, 0xff));
}


void
EnvelopePlot::customEvent (QEvent* event)
{
	if (dynamic_cast<UpdateRequest*> (event))
		update();
}

} // namespace Haruhi

