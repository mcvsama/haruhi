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

#ifndef HARUHI__PLUGINS__POLYTRAINER__PIANO_WIDGET_H__INCLUDED
#define HARUHI__PLUGINS__POLYTRAINER__PIANO_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtGui/QWidget>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/event.h>


namespace Polytrainer {

class PianoWidget: public QWidget
{
	enum KeyStateFlags {
		KeyDepressed	= 1 << 0,
		KeyHighlighted	= 1 << 1,
	};

	typedef int KeyState;

	static constexpr float key_width = 15.0f;
	static constexpr float key_height = 5.6 * key_width;

  public:
	PianoWidget (QWidget* parent);

	/**
	 * Set key range.
	 */
	void
	set_key_range (Haruhi::Key lowest, Haruhi::Key highest);

	/**
	 * Highlight key.
	 */
	void
	set_key_highlighted (Haruhi::Key key, bool highlighted);

	/**
	 * Clear all highlights.
	 */
	void
	reset_all_highlights();

	/**
	 * Clear all depressions.
	 */
	void
	reset_all_depressions();

	/**
	 * Depress or release key.
	 */
	void
	set_key_depressed (Haruhi::Key key, bool depressed);

  protected:
	void
	paintEvent (QPaintEvent*);

  private:
	void
	paint_key (QPainter&, Haruhi::Key key, QRectF const& size, KeyState key_state) const;

	QBrush
	key_state_to_brush (KeyState key_state, QColor default_color) const;

	float
	midpoint (float midpoint, float a, float b) const;

	float
	compute_width() const;

  private:
	std::array<bool, Haruhi::MaxKeyID + 1>	_highlighted;
	std::array<bool, Haruhi::MaxKeyID + 1>	_depressed;
	Haruhi::Key								_lowest_key;
	Haruhi::Key								_highest_key;
};

} // namespace Polytrainer

#endif

