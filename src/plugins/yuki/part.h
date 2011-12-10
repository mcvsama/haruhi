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

#ifndef HARUHI__PLUGINS__YUKI__PART_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wave.h>
#include <haruhi/dsp/wavetable.h>
#include <haruhi/graph/event.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/work_performer.h>

// Local:
#include "has_widget.h"
#include "has_id.h"
#include "params.h"


namespace Yuki {

namespace DSP = Haruhi::DSP;

class PartWidget;
class PartManager;
class VoiceManager;

class Part:
	public HasWidget<PartWidget>,
	public HasID
{
	class UpdateWavetableWorkUnit: public WorkPerformer::Unit
	{
	  public:
		/**
		 * \param	wavetable is the wavetable to update.
		 */
		UpdateWavetableWorkUnit (Part*);

		/**
		 * Prepare work unit for another work.
		 * \param	wave Wave to be used in wavetable.
		 * \param	wavetable Target wavetable object.
		 * \param	serial Update request ID.
		 */
		void
		reset (DSP::Wave* wave, DSP::Wavetable* wavetable, unsigned int serial);

		/**
		 * Recompute wavetable.
		 */
		void
		execute();

		/**
		 * Cancel computations in the middle as soon as possible.
		 */
		void
		cancel();

		/**
		 * Return serial.
		 */
		unsigned int
		serial() const;

	  private:
		/**
		 * Return true if the unit was marked as cancelled
		 * with cancel() method.
		 */
		bool
		is_cancelled() const;

	  private:
		Part*			_part;
		DSP::Wave*		_wave;
		DSP::Wavetable*	_wavetable;
		unsigned int	_serial;
		Atomic<bool>	_is_cancelled;
	};

  public:
	Part (PartManager*, WorkPerformer* rendering_work_performer);

	~Part();

	/**
	 * Handle voice input event.
	 * Pass it to the voice manager if conditions are met.
	 */
	void
	handle_voice_event (Haruhi::VoiceEvent const*);

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
	 * Update wavetable according to new parameters.
	 * Switch double-buffered wavetables.
	 * Do not propagate new wavetable to VoiceManager
	 */
	void
	update_wavetable();

	/**
	 * Start voices rendering.
	 */
	void
	render();

	/**
	 * Wait until voice rendering is done.
	 */
	void
	wait_for_render();

	/**
	 * Mix rendered voices into given buffers.
	 */
	void
	mix_rendering_result (Haruhi::AudioBuffer*, Haruhi::AudioBuffer*);

	/**
	 * Return voices number generated by this part.
	 */
	unsigned int
	voices_number() const;

  private:
	/**
	 * Check if work unit for wavetable update
	 * is finished. Ensure that update actually
	 * takes place if needed.
	 *
	 * Called on each processing round.
	 */
	void
	check_wavetable_update_process();

	/**
	 * Switch double buffered wavetables.
	 * Propagate notification to the voice manager.
	 *
	 * \entry	UpdateWavetableWorkUnit (WorkPerformer)
	 */
	void
	switch_wavetables();

  private:
	PartManager*				_part_manager;
	VoiceManager*				_voice_manager;
	Params::Part				_params;
	DSP::Wavetable*				_wavetables[2];
	Atomic<bool>				_switch_wavetables;
	Atomic<unsigned int>		_wt_update_request;
	Atomic<unsigned int>		_wt_serial;
	UpdateWavetableWorkUnit*	_wt_wu;
	bool						_wt_wu_ever_started;
};


typedef std::list<Part*>  Parts;


inline void
Part::UpdateWavetableWorkUnit::cancel()
{
	_is_cancelled.store (true);
}


inline unsigned int
Part::UpdateWavetableWorkUnit::serial() const
{
	return _serial;
}


inline bool
Part::UpdateWavetableWorkUnit::is_cancelled() const
{
	return _is_cancelled.load();
}

} // namespace Yuki

#endif

