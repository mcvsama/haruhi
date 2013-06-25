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
#include <vector>
#include <algorithm>

// Qt:
#include <QtGui/QLayout>
#include <QtGui/QPainter>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "piano_widget.h"


namespace Polytrainer {

PianoWidget::PianoWidget (QWidget* parent):
	QWidget (parent)
{
	setAttribute (Qt::WA_NoBackground);
	setFixedHeight (key_height);
	std::fill (_highlighted.begin(), _highlighted.end(), false);
	std::fill (_depressed.begin(), _depressed.end(), false);
	set_key_range (Haruhi::Key::lowest(), Haruhi::Key::highest());
}


void
PianoWidget::set_key_range (Haruhi::Key lowest, Haruhi::Key highest)
{
	_lowest_key = lowest;
	_highest_key = highest;
	setMinimumWidth (compute_width());
}


void
PianoWidget::set_enabled_range (Haruhi::Key lowest_enabled, Haruhi::Key highest_enabled)
{
	_lowest_enabled = lowest_enabled;
	_highest_enabled = highest_enabled;
}


void
PianoWidget::set_key_highlighted (Haruhi::Key key, bool highlighted)
{
	_highlighted[bound<Haruhi::KeyID> (key.id(), 0, _highlighted.size() - 1)] = highlighted;
	update();
}


void
PianoWidget::reset_all_highlights()
{
	std::fill (_highlighted.begin(), _highlighted.end(), false);
	update();
}


void
PianoWidget::reset_all_depressions()
{
	std::fill (_depressed.begin(), _depressed.end(), false);
	update();
}


void
PianoWidget::set_key_depressed (Haruhi::Key key, bool depressed)
{
	_depressed[bound<Haruhi::KeyID> (key.id(), 0, _depressed.size() - 1)] = depressed;
	update();
}


void
PianoWidget::paintEvent (QPaintEvent*)
{
	QPixmap pixmap (size());
	QPainter painter (&pixmap);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);
	painter.fillRect (rect(), QBrush (palette().color (QPalette::Window)));

	std::vector<float> positions {
		0.f,
			0.f,
		1.f,
			1.f,
		2.f,
		3.f,
			3.f,
		4.f,
			4.f,
		5.f,
			5.f,
		6.f
	};

	float first_key_position = _lowest_key.id() % 12;
	for (Haruhi::Key k = _lowest_key; k <= _highest_key; ++k)
	{
		int km = k.id() % 12;
		int in = k.id() / 12 - _lowest_key.id() / 12;
		KeyState key_state = 0;
		if (_depressed[k.id()])
			key_state |= KeyDepressed;
		if (_highlighted[k.id()])
			key_state |= KeyHighlighted;
		float c = std::round ((width() - compute_width()) / 2.f);
		paint_key (painter, k, QRectF (QPointF (c + 0.5f + key_width * (positions[km] - positions[first_key_position] + 7 * in), 0.5f), QSizeF (key_width, height() - 1)), key_state);
	}

	QPainter (this).drawPixmap (QPoint (0, 0), pixmap, pixmap.rect());
}


void
PianoWidget::paint_key (QPainter& painter, Haruhi::Key key, QRectF const& rect, KeyState key_state) const
{
	QPolygonF shp;
	float y1 = midpoint (0.6f, rect.top(), rect.bottom());

	auto paint_white_key = [&]() -> void {
		painter.setBrush (key_state_to_brush (key_state, Qt::white));
		painter.setPen (QPen (Qt::black, 1.0f));
		painter.drawPolygon (shp);
	};

	auto paint_black_key = [&]() -> void {
		painter.setBrush (key_state_to_brush (key_state, Qt::black));
		painter.setPen (QPen (Qt::black, 1.0f));
		painter.drawPolygon (shp);
	};

	auto paint_key_left = [&] (float m1) -> void {
		float x1 = midpoint (m1, rect.left(), rect.right());
		if (key == _highest_key)
			x1 = rect.right();
		shp << rect.topLeft()
			<< QPointF (x1, rect.top())
			<< QPointF (x1, y1)
			<< QPointF (rect.right(), y1)
			<< rect.bottomRight()
			<< rect.bottomLeft()
			<< rect.topLeft();
		paint_white_key();
	};

	auto paint_key_right = [&] (float m1) -> void {
		float x1 = midpoint (m1, rect.left(), rect.right());
		if (key == _lowest_key)
			x1 = rect.left();
		shp << QPointF (x1, rect.top())
			<< rect.topRight()
			<< rect.bottomRight()
			<< rect.bottomLeft()
			<< QPointF (rect.left(), y1)
			<< QPointF (x1, y1)
			<< QPointF (x1, rect.top());
		paint_white_key();
	};

	auto paint_key_mid = [&] (float m1, float m2) -> void {
		if (key == _lowest_key)
			paint_key_left (m2);
		else if (key == _highest_key)
			paint_key_right (m1);
		else
		{
			float x1 = midpoint (m1, rect.left(), rect.right());
			float x2 = midpoint (m2, rect.left(), rect.right());
			shp << QPointF (x1, rect.top())
				<< QPointF (x2, rect.top())
				<< QPointF (x2, y1)
				<< QPointF (rect.right(), y1)
				<< rect.bottomRight()
				<< rect.bottomLeft()
				<< QPointF (rect.left(), y1)
				<< QPointF (x1, y1)
				<< QPointF (x1, rect.top());
			paint_white_key();
		}
	};

	auto paint_key_black = [&] (float m1, float m2) -> void {
		float x1 = midpoint (m1, rect.left(), rect.right());
		float x2 = midpoint (m2, rect.left(), rect.right());
		shp << QPointF (x1, rect.top())
			<< QPointF (x2, rect.top())
			<< QPointF (x2, y1)
			<< QPointF (x1, y1)
			<< QPointF (x1, rect.top());
		paint_black_key();
	};

	Haruhi::KeyID key_id = key.id();
	// C:
	if (key_id % 12 == 0)
		paint_key_left (0.7f);
	// C#:
	else if (key_id % 12 == 1)
		paint_key_black (0.7f, 1.2f);
	// D:
	else if (key_id % 12 == 2)
		paint_key_mid (0.2f, 0.8f);
	// D#:
	else if (key_id % 12 == 3)
		paint_key_black (0.8f, 1.3f);
	// E:
	else if (key_id % 12 == 4)
		paint_key_right (0.3f);
	// F:
	else if (key_id % 12 == 5)
		paint_key_left (0.7f);
	// F#:
	else if (key_id % 12 == 6)
		paint_key_black (0.7f, 1.2f);
	// G, A:
	else if (key_id % 12 == 7 || key_id % 12 == 9)
		paint_key_mid (0.25f, 0.75f);
	// G#:
	else if (key_id % 12 == 8)
		paint_key_black (0.75f, 1.25f);
	// A#:
	else if (key_id % 12 == 10)
		paint_key_black (0.8f, 1.3f);
	// B:
	else if (key_id % 12 == 11)
		paint_key_right (0.3);
}


QBrush
PianoWidget::key_state_to_brush (KeyState key_state, QColor default_color) const
{
	bool depressed = key_state & KeyDepressed;
	bool highlighed = key_state & KeyHighlighted;

	if (depressed && highlighed)
		return QBrush (QColor (0xa7, 0x6b, 0xc7));
	else if (depressed)
		return QBrush (QColor (0xc7, 0x6b, 0x6c));
	else if (highlighed)
		return QBrush (QColor (0x6b, 0x85, 0xc7));
	else
		return QBrush (default_color);
}


float
PianoWidget::midpoint (float midpoint, float a, float b) const
{
	return midpoint * (b - a) + a;
}


float
PianoWidget::compute_width() const
{
	if (_highest_key < _lowest_key)
		return 0.f;
	float width = 0;
	for (Haruhi::Key k = _lowest_key; k <= _highest_key; ++k)
	{
		// White keys:
		int m = k.id() % 12;
		if (m == 0 || m == 2 || m == 4 || m == 5 || m == 7 || m == 9 || m == 11)
			width += key_width;
	}
	return width + 1.0;
}

} // namespace Polytrainer

