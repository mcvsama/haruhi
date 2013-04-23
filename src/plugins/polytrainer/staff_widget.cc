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
#include <haruhi/utility/exception.h>

// Local:
#include "staff_widget.h"


namespace Polytrainer {

StaffWidget::Metrics::Metrics()
{ }


StaffWidget::Metrics::Metrics (QRectF const& staff_rect, Haruhi::Key center_note):
	_staff_rect (staff_rect),
	_center_note (center_note)
{
	if (!_center_note.is_white())
		throw Exception ("must have non-accidental note as center note");
}


inline QRectF
StaffWidget::Metrics::staff_rect() const
{
	return _staff_rect;
}


std::vector<Haruhi::KeyID>
StaffWidget::Metrics::staff_key_ids() const
{
	return _staff_key_ids;
}


float
StaffWidget::Metrics::line_y_position (Haruhi::Key key) const
{
	return _staff_rect.center().y() - _staff_rect.height() / 30.0 * _center_note.white_keys_to (key);
}


QRectF
StaffWidget::Metrics::note_rect() const
{
	float h = _staff_rect.height() / 15.0;
	QRectF r (0.f, 0.f, 1.8 * h, h);
	return r.translated (-r.width() / 2.0, -r.height() / 2.0);
}


QRectF
StaffWidget::Metrics::treble_clef_rect() const
{
	float x = _staff_rect.left() + _staff_rect.height() / 15.0;
	float h = line_y_position (Haruhi::Key (0)) - line_y_position (Haruhi::Key (1));
	return QRectF (x, line_y_position (Haruhi::Key (84)), 5.0 * h, 15.25 * h);
}


QRectF
StaffWidget::Metrics::bass_clef_rect() const
{
	float x = _staff_rect.left() + _staff_rect.height() / 15.0;
	float h = line_y_position (Haruhi::Key (0)) - line_y_position (Haruhi::Key (1));
	return QRectF (x, line_y_position (Haruhi::Key (44)), 5.0 * h, 6.0 * h);
}


std::vector<Haruhi::Key>
StaffWidget::Metrics::needs_staff_lines (Haruhi::Key note) const
{
	std::vector<Haruhi::Key> result;
	Haruhi::Key key_28 (28);
	Haruhi::Key key_48 (48);
	Haruhi::Key key_54 (54);
	Haruhi::Key key_60 (60);
	Haruhi::Key key_81 (81);

	if (note <= key_28)
	{
		for (Haruhi::Key k = note; k <= key_28; ++k)
			if (k.id() == 28 || k.id() == 24 || k.id() == 21 || k.id() == 17)
				result.push_back (k);
	}
	else if (key_48 <= note && note <= key_54)
	{
		for (Haruhi::Key k = key_48; k <= note; ++k)
			if (k.id() == 48 || k.id() == 52)
				result.push_back (k);
	}
	else if (key_54 <= note && note <= key_60)
	{
		for (Haruhi::Key k = note; k <= key_60; ++k)
			if (k.id() == 57 || k.id() == 60)
				result.push_back (k);
	}
	else if (key_81 <= note)
	{
		for (Haruhi::Key k = key_81; k <= note; ++k)
			if (k.id() == 81 || k.id() == 84 || k.id() == 88 || k.id() == 91)
				result.push_back (k);
	}

	return result;
}


std::pair<Haruhi::Key, StaffWidget::Accidental>
StaffWidget::Metrics::align_note (Haruhi::Key note)
{
	return { note, Accidental::None };
}


StaffWidget::StaffWidget (QWidget* parent):
	QWidget (parent)
{
	setAttribute (Qt::WA_NoBackground);
	setFixedHeight (20 * line_height);

	QString dir (HARUHI_PREFIX "/src/plugins/polytrainer/share");

	_clef_treble.load (dir + "/clef-treble.svg");
	_clef_bass.load (dir + "/clef-bass.svg");
	_note_1.load (dir + "/note-1.svg");
	_note_2.load (dir + "/note-2.svg");
	_note_4.load (dir + "/note-4.svg");

	update_metrics();
}


void
StaffWidget::set_note (Haruhi::Key const& key)
{
	_notes.clear();
	if (16 <= key.id() && key.id() <= 93)
		_notes.insert (key);
	update();
}


void
StaffWidget::resizeEvent (QResizeEvent*)
{
	update_metrics();
}


void
StaffWidget::paintEvent (QPaintEvent*)
{
	QPixmap pixmap (size());
	QPainter painter (&pixmap);
	painter.setRenderHint (QPainter::Antialiasing, true);
	painter.setRenderHint (QPainter::TextAntialiasing, true);
	painter.setRenderHint (QPainter::SmoothPixmapTransform, true);
	painter.setRenderHint (QPainter::NonCosmeticDefaultPen, true);
	painter.fillRect (rect(), QBrush (Qt::white));

	paint_staff (painter);

	QPainter (this).drawPixmap (QPoint (0, 0), pixmap, pixmap.rect());
}


void
StaffWidget::paint_staff (QPainter& painter)
{
	QPen staff_line_pen (Qt::black, 2.f, Qt::SolidLine, Qt::FlatCap);
	// Staff lines:
	painter.setPen (staff_line_pen);
	for (Haruhi::KeyID k_id: _metrics.staff_key_ids())
	{
		Haruhi::Key k (k_id);
		float y = _metrics.line_y_position (k);
		painter.drawLine (QPointF (_metrics.staff_rect().left(), y), QPointF (_metrics.staff_rect().right(), y));
	}
	painter.setPen (QPen (Qt::black, 3.f, Qt::SolidLine, Qt::FlatCap));
	QPointF c (1.5f, 0.f);
	float left = _metrics.staff_rect().left() - 0.5;
	painter.drawLine (QPointF (left, _metrics.line_y_position (Haruhi::Key (77))) + c,
					  QPointF (left, _metrics.line_y_position (Haruhi::Key (31))) + c);
	// Clefs:
	_clef_treble.render (&painter, QRectF (_metrics.treble_clef_rect()));
	_clef_bass.render (&painter, QRectF (_metrics.bass_clef_rect()));
	// Notes:
	painter.setPen (staff_line_pen);
	for (Haruhi::Key note: _notes)
	{
		auto aligned_note = _metrics.align_note (note);
		for (Haruhi::Key line: _metrics.needs_staff_lines (aligned_note.first))
		{
			painter.resetTransform();
			painter.translate (QPointF (_metrics.staff_rect().center().x(), _metrics.line_y_position (line)));
			QRectF r = _metrics.note_rect();
			float k = 0.6;
			float v = 0.55;
			painter.drawLine (QPointF (-k * r.width(), 0.0), QPointF (v * r.width(), 0.0));
		}
		painter.resetTransform();
		painter.translate (QPointF (_metrics.staff_rect().center().x(), _metrics.line_y_position (note)));
		paint_note_4 (painter);
	}
}


void
StaffWidget::paint_note_1 (QPainter& painter)
{
	_note_1.render (&painter, _metrics.note_rect());
}


void
StaffWidget::paint_note_2 (QPainter& painter)
{
	painter.save();
	_note_2.render (&painter, _metrics.note_rect());
	painter.setPen (QPen (Qt::black, 1.5));
	painter.drawLine (QPointF (9.5, -4.0), QPointF (9.5, -52.0));
	painter.restore();
}


void
StaffWidget::paint_note_4 (QPainter& painter)
{
	painter.save();
	_note_4.render (&painter, _metrics.note_rect().translated (0.0, -1.0));
	painter.setPen (QPen (Qt::black, 1.5));
	painter.drawLine (QPointF (8.5, -3.0), QPointF (8.5, -52.0));
	painter.restore();
}


void
StaffWidget::update_metrics()
{
	QRectF r = rect();
	float p = 0.025f * r.width();
	_metrics = Metrics (QRectF (QPointF (p, 0.2f * r.height()), QSizeF (r.width() - 2.f * p, 0.6f * r.height())), Haruhi::Key (55));
}

} // namespace Polytrainer

