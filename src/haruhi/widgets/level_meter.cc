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
#include <limits>

// Qt:
#include <QtGui/QApplication>
#include <QtGui/QShortcut>
#include <QtGui/QPainter>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/config/resources.h>
#include <haruhi/application/services.h>
#include <haruhi/dsp/utility.h>
#include <haruhi/utility/memory.h>
#include <haruhi/utility/atomic.h>

// Local:
#include "level_meter.h"


namespace Haruhi {

using namespace ScreenLiterals;


LevelMeter::LevelMeter (QWidget* parent, LevelMetersGroup* group, float lower_db, float upper_db):
	QWidget (parent),
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
	_peak_decounter (0),
	_decay_speed (0.15),
	_fps (30)
{
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
	setFixedWidth (1.15_screen_mm);
	setAutoFillBackground (false);
	QPalette p = palette();
	p.setColor (QPalette::Window, Qt::black);
	setPalette (p);

	// Precalculate gradient:
	QColor a;
	a.setHsv (220, 255, 255);
	QColor b;
	b.setHsv (127, 255, 255);
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
	for (Sample* s = begin; s != end; ++s)
		if (std::abs (*s) > max)
			max = std::abs (*s);
	// Compare to current value:
	if (_sample < max)
		_sample = max;
	if (_peak.load() < _sample || _peak_decounter < 0)
	{
		_peak.store (_sample);
		_peak_decounter = LevelMeter::PEAK_DECOUNTER;
	}
}


void
LevelMeter::set (Sample value)
{
	// Compare to current value:
	if (_sample < value)
		_sample = value;
	if (_peak.load() < _sample || _peak_decounter < 0)
	{
		_peak.store (_sample);
		_peak_decounter = LevelMeter::PEAK_DECOUNTER;
	}
}


void
LevelMeter::update()
{
	QWidget::update();
	if (_group)
		_group->update_peak (_peak.load());
}


void
LevelMeter::decay()
{
	_sample_prev = _sample;
	// Decay current sample:
	_sample *= std::pow (1.0f - _decay_speed, 30.0f / _fps);
	_peak_decounter -= 1;
}


void
LevelMeter::reset_peak()
{
	_sample = 0.0f;
	_peak.store (0.0f);
}


void
LevelMeter::paintEvent (QPaintEvent*)
{
	if (_prev_size != size())
	{
		repaint_bar_buffer();
		_prev_size = size();
	}

	const int w = width();
	const int h = height();
	const int k = h - _z_top + 1;

	const float sample_z = log_meter (20.0f * std::log10 (_sample_prev), _lower_db, _upper_db);
	const float peak_z = log_meter (20.0f * std::log10 (_peak.load()), _lower_db, _upper_db);

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
	from.getRgb (&r1, &g1, &b1);
	to.getRgb (&r2, &g2, &b2);

	const float r = (r2 - r1) * value + r1;
	const float g = (g2 - g1) * value + g1;
	const float b = (b2 - b1) * value + b1;

	return QColor (
		cut (r * curve (value)),
		cut (g * curve (value)),
		cut (b * curve (value))
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

	_bar_buffer = QPixmap (size());
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


LevelMetersGroup::Scale::Scale (QWidget* parent, float lower_db, float upper_db):
	QWidget (parent),
	_lower_db (lower_db),
	_upper_db (upper_db)
{
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
	setFixedWidth (4_screen_mm);
}


void
LevelMetersGroup::Scale::paintEvent (QPaintEvent*)
{
	QPainter painter (this);
	painter.setPen (palette().color (QPalette::WindowText));
	painter.setFont (Resources::small_font());

	int h = height();

	// Scale markers [dB]:
	for (float db: { 4, 0, -3, -10, -20, -30, -40, -50 })
	{
		float pos = log_meter (db, _lower_db, _upper_db) * h;
		painter.drawLine (0, h - pos, 4, h - pos);
		painter.drawText (7, h - pos + 2, QString::number (std::abs (db)));
	}
}


LevelMetersGroup::LevelMetersGroup (QWidget* parent, float lower_db, float upper_db):
	QWidget (parent),
	_lower_db (lower_db),
	_upper_db (upper_db),
	_peak_sample (0),
	_timer (0)
{
	_peak_button = new QPushButton ("-inf dB", this);
	_peak_button->setFont (Resources::small_font());
	_peak_button->setFixedHeight (2.0 * Resources::small_font().pixelSize());
	_peak_button->setFixedWidth (6_screen_mm);
	_peak_button->setToolTip ("C-r to reset");
	_peak_button_bg = _peak_button->palette().color (QPalette::Button);
	_peak_button_fg = _peak_button->palette().color (QPalette::ButtonText);

	_vector.push_back (new LevelMeter (this, this));
	_vector.push_back (new LevelMeter (this, this));

	_scale = new Scale (this, lower_db, upper_db);

	_timer = new QTimer (this);

	QObject::connect (_peak_button, SIGNAL (clicked()), this, SLOT (reset_peak()));
	QObject::connect (_timer, SIGNAL (timeout()), this, SLOT (update_meters()));
	new QShortcut (Qt::CTRL + Qt::Key_R, this, SLOT (reset_peak()));

	// Layouts:

	QHBoxLayout* meters_layout = new QHBoxLayout();
	meters_layout->setSpacing (0.3_screen_mm);
	meters_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
	for (LevelMeter* m: _vector)
		meters_layout->addWidget (m);
	meters_layout->addWidget (_scale);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addWidget (_peak_button);
	layout->addLayout (meters_layout);

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
			QPalette p = _peak_button->palette();
			p.setColor (QPalette::Button, Qt::red);
			p.setColor (QPalette::ButtonText, Qt::black);
			_peak_button->setPalette (p);
		}
	}
}


void
LevelMetersGroup::set_fps (int fps)
{
	for (LevelMeter* m: _vector)
		m->set_fps (fps);
	_timer->setInterval (1000.0 / fps);
}


void
LevelMetersGroup::set_decay_speed (float speed)
{
	for (LevelMeter* m: _vector)
		m->set_decay_speed (speed);
}


void
LevelMetersGroup::reset_peak()
{
	for (LevelMeter* m: _vector)
		m->reset_peak();
	_peak_sample = 0;
	_peak_button->setText ("-inf dB");
	QPalette p = _peak_button->palette();
	p.setColor (QPalette::Button, _peak_button_bg);
	p.setColor (QPalette::ButtonText, _peak_button_fg);
	_peak_button->setPalette (p);
}


void
LevelMetersGroup::update_meters()
{
	for (LevelMeter* m: _vector)
	{
		m->decay();
		m->update();
	}
}

} // namespace Haruhi

