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

#ifndef HARUHI__PLUGINS__YUKI__PART_MANAGER_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/one_pole_smoother.h>
#include <haruhi/graph/audio_port.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/graph/event_buffer.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/utility/noncopyable.h>
#include <haruhi/utility/signal.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/id_allocator.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "has_widget.h"
#include "has_plugin.h"
#include "params.h"
#include "part.h"
#include "part_widget.h"
#include "part_manager_widget.h"


namespace Yuki {

using Haruhi::Unique;

class Plugin;
class PartManagerWidget;

class PartManager:
	public HasWidget<PartManagerWidget>,
	public HasPlugin,
	public SaveableState,
	public Signal::Receiver,
	private Noncopyable
{
  public:
	/**
	 * Main controls' ports.
	 */
	class MainPorts:
		public HasPlugin,
		private Noncopyable
	{
	  public:
		MainPorts (Plugin*);

	  public:
		// Ports (0 = L, 1 = R):
		Unique<Haruhi::AudioPort> audio_out[2];

		// Voice inputs:
		Unique<Haruhi::EventPort> voice_in;
		Unique<Haruhi::EventPort> voice_pitch;
		Unique<Haruhi::EventPort> voice_velocity;

		// Main controls:
		Unique<Haruhi::EventPort> volume;
		Unique<Haruhi::EventPort> panorama;
		Unique<Haruhi::EventPort> detune;
		Unique<Haruhi::EventPort> stereo_width;

		// Common ports that forward messages to all parts:
		Unique<Haruhi::EventPort> amplitude;
		Unique<Haruhi::EventPort> frequency;
		Unique<Haruhi::EventPort> pitchbend;
	};

	/**
	 * Main ports' ControllerProxies.
	 */
	class MainProxies: private Noncopyable
	{
	  public:
		MainProxies (MainPorts*, Params::Main* main_params);

		/**
		 * Call process_events() on each proxy.
		 */
		void
		process_events();

	  public:
		// Part:
		Haruhi::v06::ControllerProxy volume;
		Haruhi::v06::ControllerProxy panorama;
		Haruhi::v06::ControllerProxy detune;
		Haruhi::v06::ControllerProxy stereo_width;
	};

  public:
	PartManager (Plugin*);

	~PartManager();

	/**
	 * Return number of currently existing parts.
	 */
	unsigned int
	parts_number() const noexcept;

	/**
	 * Return object containing event ports.
	 */
	MainPorts*
	ports() noexcept;

	/**
	 * Return MainProxies object.
	 */
	MainProxies*
	proxies() noexcept;

	/**
	 * Return params object.
	 */
	Params::Main*
	main_params() noexcept;

	/**
	 * Add new Part.
	 * Send part_added signal.
	 */
	Part*
	add_part();

	/**
	 * Remove given Part.
	 * Send part_removed signal.
	 */
	void
	remove_part (Part*);

	/**
	 * Remove all Parts.
	 * Send part_removed signals.
	 */
	void
	remove_all_parts();

	/**
	 * Add first Part if there is none.
	 */
	void
	ensure_there_is_at_least_one_part();

	/**
	 * Moves part position in a list of parts.
	 * Position counts from 0.
	 */
	void
	set_part_position (Part*, unsigned int position);

	/**
	 * Moves part at position old_position to the new_position.
	 */
	void
	set_part_position (unsigned int old_position, unsigned int new_position);

	/**
	 * Process events.
	 */
	void
	process();

	/**
	 * Panic all voices.
	 */
	void
	panic();

	/**
	 * Resize buffers, etc.
	 */
	void
	graph_updated();

	/**
	 * Return number of voices generated by all parts.
	 */
	unsigned int
	voices_number() const;

	/*
	 * SaveableState implementation
	 */

	void
	save_state (QDomElement&) const override;

	void
	load_state (QDomElement const&) override;

  private:
	/**
	 * Called whenever oversampling parameter changes.
	 * Calls-out set_oversampling() with proper value.
	 */
	void
	oversampling_updated();

	/**
	 * Set oversampling on all parts.
	 */
	void
	set_oversampling (unsigned int oversampling);

	/**
	 * Return graph lock if graph is available (unit is registered).
	 */
	Mutex::Lock
	get_graph_lock() const;

  public:
	Signal::Emiter<Part*>	part_added;
	Signal::Emiter<Part*>	part_removed;

	/**
	 * Emitted when Part ID has changed.
	 */
	Signal::Emiter<Part*>	part_updated;

  private:
	Params::Main			_main_params;
	Parts					_parts;
	RecursiveMutex			_parts_mutex;
	IDAllocator				_id_alloc;

	MainPorts				_ports;
	MainProxies				_proxies;

	DSP::OnePoleSmoother	_volume_smoother[2];
	DSP::OnePoleSmoother	_panorama_smoother[2];
	DSP::OnePoleSmoother	_stereo_width_smoother;
};


inline unsigned int
PartManager::parts_number() const noexcept
{
	Mutex::Lock lock (_parts_mutex);
	return _parts.size();
}


inline PartManager::MainPorts*
PartManager::ports() noexcept
{
	return &_ports;
}


inline PartManager::MainProxies*
PartManager::proxies() noexcept
{
	return &_proxies;
}


inline Params::Main*
PartManager::main_params() noexcept
{
	return &_main_params;
}

} // namespace Yuki

#endif

