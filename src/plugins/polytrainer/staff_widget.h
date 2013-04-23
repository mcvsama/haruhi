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

#ifndef HARUHI__PLUGINS__POLYTRAINER__STAFF_WIDGET_H__INCLUDED
#define HARUHI__PLUGINS__POLYTRAINER__STAFF_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>
#include <utility>
#include <set>

// Qt:
#include <QtGui/QWidget>
#include <QtSvg/QSvgRenderer>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/event.h>


namespace Polytrainer {

class StaffWidget: public QWidget
{
	static constexpr float line_height = 20.f;

	enum class Accidental { None, Flat, Sharp };

	class Metrics
	{
	  public:
		// Ctor
		Metrics();

		// Ctor
		Metrics (QRectF const& staff_rect, Haruhi::Key center_note);

		QRectF
		staff_rect() const;

		std::vector<Haruhi::KeyID>
		staff_key_ids() const;

		/**
		 * \param	line Line number from 0 to 4.
		 */
		float
		line_y_position (Haruhi::Key key) const;

		QRectF
		note_rect() const;

		QRectF
		treble_clef_rect() const;

		QRectF
		bass_clef_rect() const;

		/**
		 * Return true if note needs its own micro staff line.
		 * Note must be aligned with align_note().
		 */
		std::vector<Haruhi::Key>
		needs_staff_lines (Haruhi::Key note) const;

		/**
		 * Return note aligned to music key and accidental if needed.
		 * TODO accidental
		 */
		std::pair<Haruhi::Key, Accidental>
		align_note (Haruhi::Key);

	  private:
		std::vector<Haruhi::KeyID>	_staff_key_ids = { 31, 35, 38, 41, 45, /* 43, 47, 50, 53, 57, */ 64, 67, 71, 74, 77 };
		QRectF						_staff_rect;
		Haruhi::Key					_center_note;
	};

  public:
	StaffWidget (QWidget* parent);

	void
	set_note (Haruhi::Key const&);

  protected:
	void
	resizeEvent (QResizeEvent*);

	void
	paintEvent (QPaintEvent*);

  private:
	void
	paint_staff (QPainter& painter);

	void
	paint_note_1 (QPainter& painter);

	void
	paint_note_2 (QPainter& painter);

	void
	paint_note_4 (QPainter& painter);

	void
	update_metrics();

  private:
	std::set<Haruhi::Key>	_notes;
	Metrics					_metrics;
	QSvgRenderer			_clef_treble;
	QSvgRenderer			_clef_bass;
	QSvgRenderer			_note_1;
	QSvgRenderer			_note_2;
	QSvgRenderer			_note_4;
};

} // namespace Polytrainer

#endif

