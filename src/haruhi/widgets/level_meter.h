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

#ifndef HARUHI__WIDGETS__LEVEL_METER_H__INCLUDED
#define HARUHI__WIDGETS__LEVEL_METER_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>
#include <vector>

// Qt:
#include <QTimer>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/atomic.h>


namespace Haruhi {

class LevelMetersGroup;


class LevelMeter: public QWidget
{
	enum { PEAK_DECOUNTER = 100 };

  public:
	LevelMeter (QWidget* parent, LevelMetersGroup* group = 0, float lower_db = -70.0, float upper_db = 6.0);

	~LevelMeter();

	void
	process (Sample* begin, Sample* end);

	void
	set (Sample value);

	void
	update();

	void
	decay();

	void
	reset_peak();

	/**
	 * Sets decay speed.
	 * \param	speed Decay speed: [0 (no decay)..1.0 (immediate decay)].
	 */
	void
	set_decay_speed (float speed) { _decay_speed = speed; }

	/**
	 * Tells LevelMeter at what fps it will be updated.
	 * This information is used to decay meter at correct speed.
	 */
	void
	set_fps (int fps) { _fps = fps; }

  protected:
	void
	paintEvent (QPaintEvent*) override;

  private:
	static QColor
	color_between (QColor const& from, QColor const& to, float value);

	void
	recalculate_vars();

	void
	repaint_bar_buffer();

	static float
	curve (float value)
	{
		return 1.0f + 0.4f * std::sin (renormalize (value, 0.0f, 1.0f, 0.0f, M_PI));
	}

  private:
	LevelMetersGroup*	_group;
	bool				_to_repaint_bar_buffer;

	// Meter configuration:
	float				_lower_db;
	float				_upper_db;

	// Helpers:
	QSize				_prev_size;
	QPixmap				_bar_buffer;
	int					_z_zero;
	int					_z_top;
	int					_z_peak;

	// Current meter value:
	float				_sample;
	float				_sample_prev;
	Atomic<float>		_peak;
	int					_peak_decounter;
	float				_decay_speed;
	int					_fps;

	// Colors table:
	QColor				_colors[256];
};


class LevelMetersGroup: public QWidget
{
	Q_OBJECT

	friend class LevelMeter;

	class Scale: public QWidget
	{
	  public:
		Scale (QWidget* parent, float lower_db, float upper_db);

	  protected:
		void
		paintEvent (QPaintEvent*);

	  private:
		// Meter configuration:
		float _lower_db;
		float _upper_db;
	};

	typedef std::vector<LevelMeter*>  Vector;

  public:
	LevelMetersGroup (QWidget* parent, float lower_db = -70.0, float upper_db = 6.0);

	~LevelMetersGroup();

	LevelMeter*
	meter (Vector::size_type index);

	void
	update_peak (Sample sample);

	void
	set_fps (int fps);

	/**
	 * Sets decay speed for each LevelMeter in group.
	 */
	void
	set_decay_speed (float speed);

  public slots:
	void
	reset_peak();

	void
	update_meters();

  private:
	// Meter configuration:
	float			_lower_db;
	float			_upper_db;

	// Vector of meters:
	Vector			_vector;

	// Widgets:
	Sample			_peak_sample;
	QPushButton*	_peak_button;
	Scale*			_scale;
	QColor			_peak_button_bg;
	QColor			_peak_button_fg;

	// Timer for decaying meters:
	QTimer*			_timer;
	float			_decay_speed;
};

} // namespace Haruhi

#endif

