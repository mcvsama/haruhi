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

#ifndef HARUHI__PLUGINS__YUKI__PLUGIN_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PLUGIN_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/id_allocator.h>
#include <haruhi/plugin/plugin.h>
#include <haruhi/graph/audio_port.h>
#include <haruhi/graph/event_port.h>


namespace Yuki {

class PartManager;
class PartManagerWidget;
class WaveComputer;

class Plugin: public Haruhi::Plugin
{
	Q_OBJECT

  public:
	Plugin (std::string const& urn, std::string const& title, int id, QWidget* parent);

	virtual ~Plugin();

	WaveComputer*
	wave_computer() const { return _wave_computer; }

	/*
	 * Plugin implementation.
	 */

	void
	registered();

	void
	unregistered();

	void
	process();

	void
	panic();

	void
	graph_updated();

  private:
	PartManager*		_part_manager;
	PartManagerWidget*	_part_manager_widget;
	WaveComputer*		_wave_computer;

	// Ports (0 = L, 1 = R):
	Haruhi::AudioPort*	_audio_out[2];
	Haruhi::EventPort*	_voice_in;
};

} // namespace Yuki

#endif

