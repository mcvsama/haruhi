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

#ifndef HARUHI__UNITS__MIKURU__WAVESHAPER_H__INCLUDED
#define HARUHI__UNITS__MIKURU__WAVESHAPER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/core/audio_buffer.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "widgets.h"
#include "params.h"


class Mikuru;

namespace MikuruPrivate {

class Part;

class Waveshaper: public QWidget
{
	Q_OBJECT

	friend class Patch;

  public:
	Waveshaper (Part* part, Haruhi::PortGroup* port_group, QString const& port_prefix, Mikuru* mikuru, QWidget* parent);

	~Waveshaper();

	Params::Waveshaper*
	params() { return &_params; }

	void
	process (Haruhi::AudioBuffer* buffer1, Haruhi::AudioBuffer* buffer2);

  public slots:
	/**
	 * Loads widgets values from Params struct.
	 */
	void
	load_params();

	/**
	 * Updates Params structure from widgets.
	 */
	void
	update_params();

	/**
	 * Updates widgets.
	 */
	void
	update_widgets();

  private:
	Mikuru*				_mikuru;
	Part*				_part;
	Params::Waveshaper	_params;
	bool				_loading_params;

	// Knobs:
	Haruhi::Knob*		_control;
	int					_i;// TODO temporary
};

} // namespace MikuruPrivate

#endif

