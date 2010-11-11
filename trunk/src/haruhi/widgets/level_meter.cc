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
#include <limits>

// Qt:
#include <QtGui/QApplication>
#include <QtGui/QShortcut>
#include <QtGui/QPainter>
#include <QtGui/QToolTip>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/utility/memory.h>
#include <haruhi/dsp/utility.h>

// Local:
#include "level_meter.h"


namespace Haruhi {

LevelMeter::LevelMeter (QWidget* parent, LevelMetersGroup* group, float lower_db, float upper_db, const char* name):
	QWidget (parent, name, Qt::WNoAutoErase),
	_group (group),
	_to_repaint_bar_buffer (false),
	_lower_db (lower_db),
	_upper_db (upper_db),
	_z_zero (0),
	_z_top (0),
	_z_peak (0),
	_sample (0),
	_sample_prev (0),
	_peak (0),
	_peak_decounter (0)
{
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
	setFixedWidth (5);
	setBackgroundColor (QColor (0, 0, 0));

	// Precalculate gradient:
	const QColor a = QColor (220, 255, 255, QColor::Hsv);
	const QColor b = QColor (127, 255, 255, QColor::Hsv);
	for (int i = 0; i < 256; ++i)
		_colors[i] = LevelMeter::color_between (a, b, static_cast<float> (i) / (256));

	repaint_bar_buffer();
}


LevelMeter::~LevelMeter()
{
}


void
LevelMeter::process (Sample* begin, Sample* end)
{
	// Find maximum:
	Sample register max = 0;
	for (Sample* s = begin; s != end;  ++s)
		if (std::abs (*s) > max)
			max = std::abs (*s);
	// Compare to current value:
	if (_sample < max)
		_sample = max;
	if (_peak < _sample || _peak_decounter < 0)
	{
		_peak = _sample;
		_peak_decounter = LevelMeter::PEAK_DECOUNTER;
	}
}


void
LevelMeter::update()
{
	QWidget::update();
	if (_group)
		_group->update_peak (_peak);
}


void
LevelMeter::decay()
{
	_sample_prev = _sample;
	// Decay current sample:
	_sample *= 0.85;
	_peak_decounter -= 1;
}


void
LevelMeter::reset_peak()
{
	_sample = 0.0;
	_peak = 0.0;
}


void
LevelMeter::paintEvent (QPaintEvent* event)
{
	if (_prev_size != size())
	{
		repaint_bar_buffer();
		_prev_size = size();
	}
 
	const int w = width();
	const int h = height();
	const int k = h - _z_top + 1;

	const float sample_z = log_meter (20 * std::log10 (_sample_prev), _lower_db, _upper_db);
	const float peak_z = log_meter (20 * std::log10 (_peak), _lower_db, _upper_db);

	_z_top = h * sample_z;
	_z_peak = h * peak_z;

	QPainter painter (this);
	// Black:
	painter.fillRect (0, 0, w, k, QBrush (QColor (0, 0, 0), Qt::SolidPattern));
	// Bar:
	painter.drawPixmap (0, k, w, h - k, _bar_buffer, 0, k, w, h - k);
	// Peak:
	if (_z_peak > 30)
		painter.fillRect (0, h - _z_peak - 1, w, 3, QBrush ((_z_peak > _z_zero) ? QColor (0xff, 0x00, 0x00) : _colors[255 * _z_peak / _z_zero]));
}


QColor
LevelMeter::color_between (QColor const& from, QColor const& to, float value)
{
	int r1, g1, b1;
	int r2, g2, b2;
	from.rgb (&r1, &g1, &b1);
	to.rgb (&r2, &g2, &b2);

	const float r = (r2 - r1) * value + r1;
	const float g = (g2 - g1) * value + g1;
	const float b = (b2 - b1) * value + b1;

	return QColor (
		cut (r * curve (value)),
		cut (g * curve (value)),
		cut (b * curve (value)),
		QColor::Rgb
	);
}


void
LevelMeter::recalculate_vars()
{
	const int h = height();

	// Calculate values:
	_z_zero = h * log_meter (0, _lower_db, _upper_db);
}


void
LevelMeter::repaint_bar_buffer()
{
	const int w = width();
	const int h = height();

	recalculate_vars();

	_bar_buffer.resize (w, h);
	QPainter painter (&_bar_buffer);

	// Green zone:
	for (int z = 0; z <= _z_zero; ++z)
	{
		painter.setPen (_colors[255 * z / _z_zero]);
		painter.drawLine (0, h - z, w, h - z);
	}

	// Red zone:
	painter.fillRect (0, 0, w, h - _z_zero, QBrush (QColor (0xff, 0x00, 0x00)));
}


LevelMetersGroup::Scale::Scale (QWidget* parent, float lower_db, float upper_db, const char* name):
	QWidget (parent, name),
	_lower_db (lower_db),
	_upper_db (upper_db)
{
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
	setFixedWidth (16);
}


void
LevelMetersGroup::Scale::paintEvent (QPaintEvent* event)
{
	QPainter painter (this);
	painter.setPen (QColorGroup::Text);
	painter.setFont (Config::small_font);

	int h = height();

	// Scale markers [dB]:
	float dbs[] = { 4, 0, -3, -10, -20, -30, -40, -50 };

	for (float* db = dbs;  db != dbs + sizeof dbs / sizeof *dbs;  ++db)
	{
		float pos = log_meter (*db, _lower_db, _upper_db) * h;
		painter.drawLine (0, h - pos, 4, h - pos);
		painter.drawText (7, h - pos + 2, QString::number (std::abs (*db)));
	}
}


LevelMetersGroup::LevelMetersGroup (QWidget* parent, float lower_db, float upper_db, const char* name):
	QWidget (parent, name),
	_lower_db (lower_db),
	_upper_db (upper_db),
	_peak_sample (0),
	_timer (0)
{
	QVBoxLayout* layout1 = new QVBoxLayout (this, 0, Config::spacing);

		_peak_button = new QPushButton ("-inf dB", this);
		_peak_button->setFont (Config::small_font);
		_peak_button->setFixedHeight (2 * Config::small_font.pointSize());
		_peak_button->setFixedWidth (35);
		_peak_button_bg = _peak_button->paletteBackgroundColor();
		_peak_button_fg = _peak_button->paletteForegroundColor();

	layout1->addWidget (_peak_button);

		QHBoxLayout* layout2 = new QHBoxLayout (layout1, 1);

			_vector.push_back (new LevelMeter (this, this));
			_vector.push_back (new LevelMeter (this, this));

			_scale = new Scale (this, lower_db, upper_db);

		layout2->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
		for (Vector::iterator m = _vector.begin();  m != _vector.end();  ++m)
			layout2->addWidget (*m);
		layout2->addWidget (_scale);

	_timer = new QTimer (this);

	QObject::connect (_peak_button, SIGNAL (clicked()), this, SLOT (reset_peak()));
	QObject::connect (_timer, SIGNAL (timeout()), this, SLOT (update_meters()));
	new QShortcut (Qt::CTRL + Qt::Key_R, this, SLOT (reset_peak()));
	QToolTip::add (_peak_button, "C-r to reset");

	_timer->start (25); // 40 fps
}


LevelMetersGroup::~LevelMetersGroup()
{
	_timer->stop();
	std::for_each (_vector.begin(), _vector.end(), delete_operator<Vector::value_type>);
}


LevelMeter*
LevelMetersGroup::meter (Vector::size_type index)
{
	return _vector[index];
}


void
LevelMetersGroup::update_peak (Sample sample)
{
	sample = std::abs (sample);
	if (_peak_sample < sample)
	{
		_peak_sample = sample;
		float db = 20 * std::log10 (sample);
		if (db < _lower_db * 2)
			db = -std::numeric_limits<float>::infinity();
		_peak_button->setText (QString::number (db, 'f', 1) + " dB");
		if (sample >= 1.0)
		{
			_peak_button->setPaletteBackgroundColor (QColor (255, 0, 0));
			_peak_button->setPaletteForegroundColor (QColor (0, 0, 0));
		}
	}
}


void
LevelMetersGroup::reset_peak()
{
	for (Vector::iterator m = _vector.begin();  m != _vector.end();  ++m)
		(*m)->reset_peak();
	_peak_sample = 0;
	_peak_button->setText ("-inf dB");
	_peak_button->setPaletteBackgroundColor (_peak_button_bg);
	_peak_button->setPaletteForegroundColor (_peak_button_fg);
}


void
LevelMetersGroup::update_meters()
{
	for (Vector::iterator m = _vector.begin();  m != _vector.end();  ++m)
	{
		(*m)->decay();
		(*m)->update();
	}
}

} // namespace Haruhi

