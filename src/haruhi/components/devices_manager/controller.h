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

#ifndef HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_H__INCLUDED
#define HARUHI__COMPONENTS__DEVICES_MANAGER__CONTROLLER_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtCore/QString>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/one_pole_smoother.h>
#include <haruhi/graph/event.h>
#include <haruhi/graph/event_buffer.h>
#include <haruhi/graph/graph.h>
#include <haruhi/lib/midi.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/shared.h>
#include <haruhi/utility/seconds.h>
#include <haruhi/utility/frequency.h>


namespace Haruhi {

namespace DevicesManager {

class Device;

/**
 * Represents controller inside device. Used to filter certain MIDI events.
 */
class Controller: public SaveableState
{
	struct SmoothingParams
	{
		SmoothingParams();

		ControllerEvent::Value	current;
		ControllerEvent::Value	target;
		DSP::OnePoleSmoother	smoother;
		Timestamp				prev_timestamp;
	};

  public:
	Controller (QString const& name = "");

	bool
	operator== (Controller const& other) const;

	/**
	 * Name for controller. Name is for UI.
	 */
	QString
	name() const;

	/**
	 * Sets new name for controller.
	 */
	void
	set_name (QString const& name);

	/**
	 * Sets filters from MIDI event and stops learning.
	 * \returns	true if learned, false if event was ignored.
	 */
	bool
	learn_from_event (MIDI::Event const& event);

	/**
	 * Interprets MIDI event and maybe adds new Haruhi Events to the buffer.
	 * \param	event Input MIDI event.
	 * \param	device Device that owns this controller.
	 * \param	buffer Buffer where new events will be added.
	 * \param	graph Graph object for audio/event context (master-tune).
	 * \returns	true if event has been handled, false otherwise.
	 */
	bool
	handle_event (MIDI::Event const& event, Device& device, EventBuffer& buffer, Graph* graph);

	/**
	 * Create and push events for value smoothing.
	 * Should be called on each processing round.
	 */
	void
	generate_smoothing_events (EventBuffer& buffer, Graph* graph);

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement&) const override;

	void
	load_state (QDomElement const&) override;

  private:
	void
	reset_filters();

	void
	controller_smoothing_setup (Timestamp t, float target, Seconds min_coeff, Seconds max_coeff, Frequency sample_rate);

	void
	channel_pressure_smoothing_setup (Timestamp t, float target, Seconds min_coeff, Seconds max_coeff, Frequency sample_rate);

	void
	key_pressure_smoothing_setup (unsigned int key, Timestamp t, float target, Seconds min_coeff, Seconds max_coeff, Frequency sample_rate);

  public:
	// MIDI filters:
	bool	note_filter					= false;
	int		note_channel				= 0;		// 0 means 'all'
	bool	note_velocity_filter		= false;
	int		note_velocity_channel		= 0;		// 0 means 'all'
	bool	note_pitch_filter			= false;
	int		note_pitch_channel			= 0;		// 0 means 'all'
	bool	controller_filter			= false;
	int		controller_channel			= 0;		// 0 means 'all'
	int		controller_number			= 0;
	bool	controller_invert			= false;
	bool	pitchbend_filter			= false;
	int		pitchbend_channel			= 0;		// 0 means 'all'
	bool	channel_pressure_filter		= false;
	int		channel_pressure_channel	= 0;		// 0 means 'all'
	bool	channel_pressure_invert		= false;
	bool	key_pressure_filter			= false;
	int		key_pressure_channel		= 0;		// 0 means 'all'
	bool	key_pressure_invert			= false;
	// Smooth controller/pressure events:
	Seconds	smoothing					= 0_ms;

  private:
	QString								_name;
	// Used for value smoothing:
	SmoothingParams						_controller_smoother;
	SmoothingParams						_channel_pressure_smoother;
	std::array<SmoothingParams, 128>	_key_pressure_smoother;
};


inline
Controller::SmoothingParams::SmoothingParams():
	current (0.f), target (0.f)
{ }


inline QString
Controller::name() const
{
	return _name;
}


inline void
Controller::set_name (QString const& name)
{
	_name = name;
}

} // namespace DevicesManager

} // namespace Haruhi

#endif

