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

#ifndef HARUHI__PLUGINS__YUKI__PART_MANAGER_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTabWidget>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/one_pole_smoother.h>
#include <haruhi/graph/audio_port.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/graph/event_buffer.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/utility/noncopyable.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/id_allocator.h>

// Local:
#include "has_widget.h"
#include "has_plugin.h"
#include "params.h"
#include "part.h"
#include "part_widget.h"


namespace Yuki {

class Plugin;

class PartManager:
	public HasWidget<PartManagerWidget>,
	public HasPlugin,
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

		~MainPorts();

	  public:
		// Ports (0 = L, 1 = R):
		Haruhi::AudioPort* audio_out[2];

		// Voice port:
		Haruhi::EventPort* voice_in;

		// Main controls:
		Haruhi::EventPort* volume;
		Haruhi::EventPort* panorama;
		Haruhi::EventPort* detune;
		Haruhi::EventPort* stereo_width;

		// Common ports that forward messages to all parts:
		Haruhi::EventPort* amplitude;
		Haruhi::EventPort* frequency;
		Haruhi::EventPort* pitchbend;
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
		Haruhi::ControllerProxy volume;
		Haruhi::ControllerProxy panorama;
		Haruhi::ControllerProxy detune;
		Haruhi::ControllerProxy stereo_width;
	};

  public:
	PartManager (Plugin*);

	~PartManager();

	/**
	 * Return reference to set of Parts used by synth.
	 */
	Parts&
	parts();

	/**
	 * Return MainProxies object.
	 */
	MainProxies*
	proxies();

	/**
	 * Add new Part.
	 */
	void
	add_part();

	/**
	 * Remove given Part.
	 */
	void
	remove_part (Part*);

	/**
	 * Remove all Parts.
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

  public:
	Signal::Emiter1<Part*>	part_added;
	Signal::Emiter1<Part*>	part_removed;

  private:
	Params::Main			_main_params;
	Parts					_parts;
	Mutex					_parts_mutex;
	IDAllocator				_id_alloc;

	MainPorts				_ports;
	MainProxies				_proxies;

	DSP::OnePoleSmoother	_volume_smoother[2];
	DSP::OnePoleSmoother	_panorama_smoother[2];
	DSP::OnePoleSmoother	_stereo_width_smoother;
};


inline Parts&
PartManager::parts()
{
	return _parts;
}


inline PartManager::MainProxies*
PartManager::proxies()
{
	return &_proxies;
}

} // namespace Yuki

#endif

