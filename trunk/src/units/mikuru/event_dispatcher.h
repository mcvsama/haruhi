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

#ifndef HARUHI__UNITS__MIKURU__EVENT_DISPATCHER_H__INCLUDED
#define HARUHI__UNITS__MIKURU__EVENT_DISPATCHER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "params.h"


class Mikuru;

namespace MikuruPrivate {

class VoiceManager;


class EventDispatcher
{
	typedef std::map<Haruhi::VoiceID, Haruhi::VoiceControllerEvent const*> VCEMap;

  public:
	/**
	 * Receiver's receive() method gets called whenever
	 * there is polyphonic parameter to set.
	 */
	class Receiver
	{
	  public:
		virtual void
		receive (Haruhi::VoiceID, int value) = 0;
	};

	/**
	 * Receiver for Voice parameters.
	 */
	class VoiceParamReceiver: public Receiver
	{
	  public:
		typedef Haruhi::ControllerParam (Params::Voice::* VoiceParam);

	  public:
		VoiceParamReceiver (VoiceManager* voice_manager, VoiceParam voice_param):
			_voice_manager (voice_manager),
			_voice_param (voice_param)
		{ }

		void
		receive (Haruhi::VoiceID, int value);

		void
		receive (int value);

	  private:
		VoiceManager*	_voice_manager;
		VoiceParam		_voice_param;
	};

	/**
	 * Receiver for Voice Filter 1/2 parameters.
	 */
	class VoiceFilterParamReceiver: public Receiver
	{
	  public:
		typedef Haruhi::ControllerParam (Params::Filter::* FilterParam);
		enum FilterID { Filter1, Filter2 };

	  public:
		VoiceFilterParamReceiver (VoiceManager* voice_manager, FilterID filter_id, FilterParam filter_param):
			_voice_manager (voice_manager),
			_filter_id (filter_id),
			_filter_param (filter_param)
		{ }

		void
		receive (Haruhi::VoiceID, int value);

		void
		receive (int value);

	  private:
		VoiceManager*	_voice_manager;
		FilterID		_filter_id;
		FilterParam		_filter_param;
	};

  public:
	/**
	 * Takes ownership of Receiver and deletes it upon destruction.
	 */
	EventDispatcher (Haruhi::EventPort* port, Haruhi::Knob* knob, Receiver* receiver);

	/**
	 * Takes ownership of Receiver and deletes it upon destruction.
	 */
	EventDispatcher (Haruhi::EventPort* port, int min, int max, Receiver* receiver);

	~EventDispatcher()
	{
		delete _receiver;
	}

	/**
	 * Loads events from assigned port and dispatches them
	 * either to Knob or directly to VoiceManager.
	 */
	void
	load_events();

  public:
	Haruhi::EventPort*	_port;
	Haruhi::Knob*		_knob;
	Receiver*			_receiver;
	VCEMap				_vcemap;
	int					_min;
	int					_max;
};

} // namespace MikuruPrivate

#endif

