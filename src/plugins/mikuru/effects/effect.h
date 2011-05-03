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

#ifndef HARUHI__UNITS__MIKURU__EFFECTS__EFFECT_H__INCLUDED
#define HARUHI__UNITS__MIKURU__EFFECTS__EFFECT_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QPushButton>

// Haruhi:
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/graph/port_group.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "../params.h"
#include "effect.h"


class Mikuru;

namespace MikuruPrivate {

class Effect: public QWidget
{
	Q_OBJECT

  public:
	Effect (int id, QString const& id_group, Mikuru* mikuru, QString const& port_group_name, Params::Effect* params, QWidget* parent);

	virtual ~Effect();

	int
	id() const { return _id; }

	bool
	enabled() { return _params->enabled.get(); }

	Params::Effect*
	params() const { return _params; }

	Haruhi::Knob*
	knob_wet() const { return _knob_wet; }

	Haruhi::Knob*
	knob_panorama() const { return _knob_panorama; }

	virtual void
	graph_updated() = 0;

	virtual void
	process_events();

	virtual void
	process (Haruhi::AudioBuffer* in1, Haruhi::AudioBuffer* in2, Haruhi::AudioBuffer* out1, Haruhi::AudioBuffer* out2) = 0;

	virtual void
	panic() = 0;

  public slots:
	/**
	 * Loads widgets values from Params struct.
	 */
	virtual void
	load_params();

	/**
	 * Updates Params structure from widgets.
	 */
	virtual void
	update_params();

  protected:
	Haruhi::PortGroup*
	port_group() const { return _port_group; }

	QWidget*
	parent_widget() const { return _effect_panel; }

  private:
	Mikuru*				_mikuru;
	QString				_id_group;
	int					_id;
	Params::Effect*		_params;

	QWidget*			_effect_panel;
	QPushButton*		_enabled_button;

	Haruhi::Knob*		_knob_wet;
	Haruhi::Knob*		_knob_panorama;

	Haruhi::PortGroup*	_port_group;
	Haruhi::EventPort*	_port_wet;
	Haruhi::EventPort*	_port_panorama;
};

} // namespace MikuruPrivate

#endif

