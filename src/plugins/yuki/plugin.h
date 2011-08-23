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


namespace Yuki {

class PartManager;
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
	/**
	 * This method will sync all inputs except
	 * those manually synced (keyboard and sustain).
	 */
	void
	sync_some_inputs();

	/**
	 * Stops all synthesizer threads.
	 */
	void
	stop_threads();

	void
	process_voice_events();

	void
	process_controller_events();

	void
	process_voices();

	void
	process_parts();

	void
	add_to_load (uint64_t microseconds);

	void
	reset_load();

	float
	current_load();

  private:
	PartManager*	_part_manager;
	WaveComputer*	_wave_computer;
};

} // namespace Yuki

#endif

