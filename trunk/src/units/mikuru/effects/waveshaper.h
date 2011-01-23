/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__UNITS__MIKURU__EFFECTS__WAVESHAPER_H__INCLUDED
#define HARUHI__UNITS__MIKURU__EFFECTS__WAVESHAPER_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtCore/QMetaType>
#include <QtGui/QWidget>
#include <QtGui/QComboBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_port.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "../params.h"
#include "effect.h"


class Mikuru;

namespace MikuruPrivate {

using Haruhi::Sample;
class Patch;

class Shaper
{
  public:
	typedef Sample (*Function)(Sample, Sample);

	Shaper():
		function (0), has_parameter (false)
	{ }

	Shaper (Shaper const& other):
		function (other.function),
		has_parameter (other.has_parameter)
	{ }

	Shaper (Function function, bool has_parameter):
		function (function),
		has_parameter (has_parameter)
	{ }

	Function	function;
	bool		has_parameter;
};


class Waveshaper: public Effect
{
	Q_OBJECT

	friend class Patch;

	typedef std::vector<Shaper*> Shapers;

  public:
	Waveshaper (int id, Mikuru* mikuru, QWidget* parent);

	~Waveshaper();

	Params::Waveshaper*
	params() { return &_params; }

	/*
	 * Effect API
	 */

	int
	id() const { return _id; }

	void
	process (Haruhi::AudioBuffer* buffer, unsigned int channel);

  private:
	/*
	 * Effect API
	 */

	Params::Effect&
	effect_params() { return _params; }

  public slots:
	/**
	 * Loads widgets values from Params struct.
	 */
	void
	load_params();

	void
	load_params (Params::Waveshaper& params);

	/**
	 * Updates Params structure from widgets.
	 */
	void
	update_params();

  private slots:
	void
	set_type (int type);

	void
	update_widgets();

  private:
	Mikuru*				_mikuru;
	int					_id;
	Params::Waveshaper	_params;
	bool				_loading_params;

	Shapers				_shapers;
	QComboBox*			_waveshaper_type_combo;
	Atomic<Shaper*>		_waveshaper_type;

	Haruhi::Knob*		_knob_gain;
	Haruhi::Knob*		_knob_parameter;

	Haruhi::PortGroup*	_port_group;
	Haruhi::EventPort*	_port_gain;
	Haruhi::EventPort*	_port_parameter;
};

} // namespace MikuruPrivate

// Qt sucks rather hardly. This must be in global namespace:
Q_DECLARE_METATYPE (MikuruPrivate::Shaper*);

#endif

