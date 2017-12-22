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

#ifndef HARUHI__PLUGINS__YUKI__PART_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_H__INCLUDED

// Standard:
#include <cstddef>
#include <iterator>
#include <list>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/dsp/wavetable.h>
#include <haruhi/dsp/parametric_wave.h>
#include <haruhi/dsp/crossing_wave.h>
#include <haruhi/graph/event.h>
#include <haruhi/graph/event_port.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/graph/port_group.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/signal.h>
#include <haruhi/utility/work_performer.h>
#include <haruhi/utility/numeric.h>
#include <haruhi/utility/noncopyable.h>
#include <haruhi/utility/saveable_state.h>

// Local:
#include "has_widget.h"
#include "has_id.h"
#include "has_plugin.h"
#include "params.h"
#include "voice_manager.h"


namespace Yuki {

namespace DSP = Haruhi::DSP;
using Haruhi::Unique;

class PartWidget;
class PartManager;
class VoiceManager;

class Part:
	public HasWidget<PartWidget>,
	public HasID,
	public HasPlugin,
	public Signal::Receiver,
	public SaveableState,
	private Noncopyable
{
	friend class PartWidget;

	class UpdateWavetableWorkUnit: public WorkPerformer::Unit
	{
	  public:
		/**
		 * \param	wavetable is the wavetable to update.
		 */
		UpdateWavetableWorkUnit (Part*) noexcept;

		/**
		 * Prepare work unit for another work.
		 * \param	base_wave Base wave for computation. It's cloned inside.
		 * \param	modulator_wave Modulator wave. It's cloned.
		 * \param	wavetable Target wavetable object.
		 * \param	serial Update request ID.
		 */
		void
		reset (DSP::Wavetable* wavetable, unsigned int serial) noexcept;

		/**
		 * Recompute wavetable.
		 */
		void
		execute() override;

		/**
		 * Cancel computations in the middle as soon as possible.
		 */
		void
		cancel() noexcept;

		/**
		 * Return serial.
		 */
		unsigned int
		serial() const noexcept;

	  private:
		/**
		 * Return true if the unit was marked as cancelled
		 * with cancel() method.
		 */
		bool
		is_cancelled() const;

	  private:
		Part*					_part;
		DSP::Wavetable*			_wavetable		= nullptr;
		unsigned int			_serial;
		Atomic<bool>			_is_cancelled;
	};

	/**
	 * Helper class that forwards Voice parameter (given by pointer to member) update notification
	 * to the voice manager (which updates voice(s)).
	 */
	template<class ParamPtr>
		class VoiceParamUpdater:
			public Signal::Receiver,
			private Noncopyable
		{
		  public:
			VoiceParamUpdater (VoiceManager* voice_manager, ParamPtr param_ptr) noexcept;

			void
			handle_change (int value);

			void
			handle_event (Haruhi::VoiceControllerEvent const* event, int value);

		  private:
			VoiceManager*	_voice_manager;
			ParamPtr		_param_ptr;
		};

	/**
	 * Same as VoiceParamUpdater, but for filter parameters.
	 */
	template<class ParamPtr>
		class FilterParamUpdater:
			public Signal::Receiver,
			private Noncopyable
		{
		  public:
			/**
			 * \param	filter_no Filter identifier: 0 or 1.
			 */
			FilterParamUpdater (VoiceManager* voice_manager, unsigned int filter_no, ParamPtr param_ptr) noexcept;

			void
			handle_change (int value);

			void
			handle_event (Haruhi::VoiceControllerEvent const* event, int value);

		  private:
			VoiceManager*	_voice_manager;
			unsigned int	_filter_no;
			ParamPtr		_param_ptr;
		};

  public:
	/**
	 * Contains Haruhi ports created for the part.
	 */
	class PartPorts: private Noncopyable
	{
	  public:
		// [0..3][0..2] -- [controlled][controlling]:
		typedef std::vector<std::vector<Unique<Haruhi::EventPort>>> MatrixPorts;

	  public:
		PartPorts (Plugin*, unsigned int part_id);

	  public:
		// Port group:
		Unique<Haruhi::PortGroup>	port_group;

		// Waveform ports:
		Unique<Haruhi::EventPort>	wave_shape;
		Unique<Haruhi::EventPort>	modulator_amplitude;
		Unique<Haruhi::EventPort>	modulator_index;
		Unique<Haruhi::EventPort>	modulator_shape;

		// Part ports:
		Unique<Haruhi::EventPort>	volume;
		Unique<Haruhi::EventPort>	portamento_time;
		Unique<Haruhi::EventPort>	phase;

		// Polyphonic-input ports:
		Unique<Haruhi::EventPort>	amplitude;
		Unique<Haruhi::EventPort>	frequency;
		Unique<Haruhi::EventPort>	panorama;
		Unique<Haruhi::EventPort>	detune;
		Unique<Haruhi::EventPort>	pitchbend;
		Unique<Haruhi::EventPort>	velocity_sens;
		Unique<Haruhi::EventPort>	unison_index;
		Unique<Haruhi::EventPort>	unison_spread;
		Unique<Haruhi::EventPort>	unison_init;
		Unique<Haruhi::EventPort>	unison_noise;
		Unique<Haruhi::EventPort>	unison_vibrato_level;
		Unique<Haruhi::EventPort>	unison_vibrato_frequency;
		Unique<Haruhi::EventPort>	noise_level;

		// Filter ports:
		Unique<Haruhi::EventPort>	filter_frequency[Params::Voice::FiltersNumber];
		Unique<Haruhi::EventPort>	filter_resonance[Params::Voice::FiltersNumber];
		Unique<Haruhi::EventPort>	filter_gain[Params::Voice::FiltersNumber];
		Unique<Haruhi::EventPort>	filter_attenuation[Params::Voice::FiltersNumber];

		// Operator ports:
		Unique<Haruhi::EventPort>	operator_detune[Params::Part::OperatorsNumber];

		// Modulator matrix:
		MatrixPorts					fm_matrix;
		MatrixPorts					am_matrix;
	};

	/**
	 * Contains ControllerProxies that connect event ports and params.
	 */
	class PartControllerProxies: private Noncopyable
	{
	  public:
		typedef std::vector<std::vector<Unique<Haruhi::v06::ControllerProxy>>> MatrixControllerProxies;

	  public:
		PartControllerProxies (PartManager*, PartPorts*, Params::Part*);

		/**
		 * Call process_events() on each proxy.
		 */
		void
		process_events();

	  private:
		/**
		 * Forward messages from the first EventPort to the second one,
		 * unless first has any back connections. Also, if neither EventPort
		 * is connected on back, make the second EventPort yield ControllerEvent
		 * with source port's default value.
		 */
		void
		forward_messages (Haruhi::EventPort* source, Haruhi::EventPort* target);

	  public:
		// Part:
		Haruhi::v06::ControllerProxy			volume;
		Haruhi::v06::ControllerProxy			portamento_time;
		Haruhi::v06::ControllerProxy			phase;
		Haruhi::v06::ControllerProxy			noise_level;
		Haruhi::v06::ControllerProxy			wave_shape;
		Haruhi::v06::ControllerProxy			modulator_amplitude;
		Haruhi::v06::ControllerProxy			modulator_index;
		Haruhi::v06::ControllerProxy			modulator_shape;

		// Voice:
		Haruhi::v06::ControllerProxy			amplitude;
		Haruhi::v06::ControllerProxy			frequency;
		Haruhi::v06::ControllerProxy			panorama;
		Haruhi::v06::ControllerProxy			detune;
		Haruhi::v06::ControllerProxy			pitchbend;
		Haruhi::v06::ControllerProxy			velocity_sens;
		Haruhi::v06::ControllerProxy			unison_index;
		Haruhi::v06::ControllerProxy			unison_spread;
		Haruhi::v06::ControllerProxy			unison_init;
		Haruhi::v06::ControllerProxy			unison_noise;
		Haruhi::v06::ControllerProxy			unison_vibrato_level;
		Haruhi::v06::ControllerProxy			unison_vibrato_frequency;

		// Filters:
		Unique<Haruhi::v06::ControllerProxy>	filter_frequency[Params::Voice::FiltersNumber];
		Unique<Haruhi::v06::ControllerProxy>	filter_resonance[Params::Voice::FiltersNumber];
		Unique<Haruhi::v06::ControllerProxy>	filter_gain[Params::Voice::FiltersNumber];
		Unique<Haruhi::v06::ControllerProxy>	filter_attenuation[Params::Voice::FiltersNumber];

		// Operators:
		Unique<Haruhi::v06::ControllerProxy>	operator_detune[Params::Part::OperatorsNumber];

		// Modulator matrix:
		MatrixControllerProxies					fm_matrix;
		MatrixControllerProxies					am_matrix;

	  private:
		PartManager* _part_manager;
	};

	/**
	 * Contains receivers of ControllerProxy notifications
	 * that voice parameters have changed.
	 * There receivers update all existing voices' params.
	 */
	class ParamUpdaters: private Noncopyable
	{
	  public:
		ParamUpdaters (VoiceManager*);

	  public:
		// Voice params updaters:
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			amplitude;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			frequency;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			panorama;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			detune;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			pitchbend;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			velocity_sens;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			unison_index;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			unison_spread;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			unison_init;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			unison_noise;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			unison_vibrato_level;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>			unison_vibrato_frequency;

		// Filter params updaters:
		Unique<FilterParamUpdater<Params::Filter::ControllerParamPtr>>	filter_frequency[Params::Voice::FiltersNumber];
		Unique<FilterParamUpdater<Params::Filter::ControllerParamPtr>>	filter_resonance[Params::Voice::FiltersNumber];
		Unique<FilterParamUpdater<Params::Filter::ControllerParamPtr>>	filter_gain[Params::Voice::FiltersNumber];
		Unique<FilterParamUpdater<Params::Filter::ControllerParamPtr>>	filter_attenuation[Params::Voice::FiltersNumber];
		Unique<FilterParamUpdater<Params::Filter::IntParamPtr>>			filter_enabled[Params::Voice::FiltersNumber];
		Unique<FilterParamUpdater<Params::Filter::IntParamPtr>>			filter_type[Params::Voice::FiltersNumber];
		Unique<FilterParamUpdater<Params::Filter::IntParamPtr>>			filter_stages[Params::Voice::FiltersNumber];
		Unique<FilterParamUpdater<Params::Filter::IntParamPtr>>			filter_limiter_enabled[Params::Voice::FiltersNumber];
	};

  public:
	Part (PartManager*, WorkPerformer* rendering_work_performer, Params::Main* main_params, unsigned int id);

	~Part();

	/**
	 * Return PartManager that manages this Part.
	 */
	PartManager*
	part_manager() const noexcept;

	/**
	 * Return part params.
	 */
	Params::Part*
	part_params() noexcept;

	/**
	 * Return ports list object.
	 */
	PartPorts*
	ports() noexcept;

	/**
	 * Return proxies list created by the part.
	 * Needed for the widget to link Knobs to proxies.
	 */
	PartControllerProxies*
	proxies() noexcept;

	/**
	 * Handle voice input event.
	 * Pass it to the voice manager if conditions are met.
	 */
	void
	handle_voice_event (Haruhi::VoiceEvent const*);

	/**
	 * Handle voice velocity event.
	 */
	void
	handle_amplitude_event (Haruhi::VoiceControllerEvent const*);

	/**
	 * Handle voice pitch event.
	 */
	void
	handle_frequency_event (Haruhi::VoiceControllerEvent const*);

	/**
	 * Process events.
	 */
	void
	process_events();

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
	 * Set oversampling for all parts.
	 * Needs Graph lock.
	 */
	void
	set_oversampling (unsigned int oversampling);

	/**
	 * Update wavetable according to new parameters.
	 * Switch double-buffered wavetables.
	 * Do not propagate new wavetable to VoiceManager - this
	 * will be done in render().
	 */
	void
	update_wavetable();

	/**
	 * Start voices rendering.
	 */
	void
	async_render();

	/**
	 * Wait until voice rendering is done.
	 */
	void
	wait_for_render();

	/**
	 * Mix rendered voices into given buffers.
	 */
	void
	mix_rendering_result (Haruhi::AudioBuffer*, Haruhi::AudioBuffer*) noexcept;

	/**
	 * Return voices number generated by this part.
	 */
	unsigned int
	voices_number() const;

	/**
	 * Return currently selected base wave object.
	 */
	DSP::ParametricWave*
	base_wave() const noexcept;

	/**
	 * Return currently selected modulator wave object.
	 */
	DSP::ParametricWave*
	modulator_wave() const noexcept;

	/**
	 * Allocate Wave* that incorporates harmonics and modulation
	 * taken from parameters object at the time of the call.
	 * Must be deleted after use.
	 * \threadsafe
	 */
	Unique<DSP::Wave>
	final_wave() const;

	/*
	 * SaveableState implementation
	 */

	void
	save_state (QDomElement&) const override;

	void
	load_state (QDomElement const&) override;

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
	 * Notify that new wavetable with given serial number
	 * has been computed.
	 *
	 * Switches wavetables and propagates notification
	 * to the voice manager.
	 *
	 * \entry	UpdateWavetableWorkUnit (WorkPerformer)
	 */
	void
	wavetable_computed (unsigned int serial);

  private:
	PartManager*					_part_manager;
	Unique<VoiceManager>			_voice_manager;
	Params::Part					_part_params;
	DSP::Wavetable					_wavetable_current;
	DSP::Wavetable::WaveAdapter		_wave_current			{ &_wavetable_current };
	DSP::Wavetable					_wavetable_next;
	DSP::Wavetable::WaveAdapter		_wave_next				{ &_wavetable_next };
	DSP::Wavetable					_wavetable_rendered;
	DSP::CrossingWave				_crossing_wave;
	DSP::Wave*						_wave					= nullptr;
	Atomic<bool>					_new_wavetable_ready	{ false };
	Atomic<unsigned int>			_wt_update_request		{ 0 };
	Atomic<unsigned int>			_wt_serial				{ 0 };
	Unique<UpdateWavetableWorkUnit>	_wt_wu;
	bool							_wt_wu_ever_started		= false;
	Unique<DSP::ParametricWave>		_modulator_waves[4];
	Unique<DSP::ParametricWave>		_base_waves[9];
	PartPorts						_ports;
	PartControllerProxies			_proxies;
	ParamUpdaters					_updaters;
};


typedef std::list<Part*>  Parts;


inline void
Part::UpdateWavetableWorkUnit::cancel() noexcept
{
	_is_cancelled.store (true);
}


inline unsigned int
Part::UpdateWavetableWorkUnit::serial() const noexcept
{
	return _serial;
}


template<class ParamPtr>
	inline
	Part::VoiceParamUpdater<ParamPtr>::VoiceParamUpdater (VoiceManager* voice_manager, ParamPtr param_ptr) noexcept:
		_voice_manager (voice_manager),
		_param_ptr (param_ptr)
	{ }


template<class ParamPtr>
	inline void
	Part::VoiceParamUpdater<ParamPtr>::handle_change (int value)
	{
		_voice_manager->update_voice_parameter (Haruhi::OmniVoice, _param_ptr, value);
	}


template<class ParamPtr>
	inline void
	Part::VoiceParamUpdater<ParamPtr>::handle_event (Haruhi::VoiceControllerEvent const* event, int value)
	{
		_voice_manager->update_voice_parameter (event->voice_id(), _param_ptr, value);
	}


template<class ParamPtr>
	inline
	Part::FilterParamUpdater<ParamPtr>::FilterParamUpdater (VoiceManager* voice_manager, unsigned int filter_no, ParamPtr param_ptr) noexcept:
		_voice_manager (voice_manager),
		_filter_no (filter_no),
		_param_ptr (param_ptr)
	{ }


template<class ParamPtr>
	inline void
	Part::FilterParamUpdater<ParamPtr>::handle_change (int value)
	{
		_voice_manager->update_filter_parameter (Haruhi::OmniVoice, _filter_no, _param_ptr, value);
	}


template<class ParamPtr>
	inline void
	Part::FilterParamUpdater<ParamPtr>::handle_event (Haruhi::VoiceControllerEvent const* event, int value)
	{
		_voice_manager->update_filter_parameter (event->voice_id(), _filter_no, _param_ptr, value);
	}


inline bool
Part::UpdateWavetableWorkUnit::is_cancelled() const
{
	return _is_cancelled.load();
}


inline PartManager*
Part::part_manager() const noexcept
{
	return _part_manager;
}


inline Params::Part*
Part::part_params() noexcept
{
	return &_part_params;
}


inline Part::PartPorts*
Part::ports() noexcept
{
	return &_ports;
}


inline Part::PartControllerProxies*
Part::proxies() noexcept
{
	return &_proxies;
}


inline DSP::ParametricWave*
Part::base_wave() const noexcept
{
	return _base_waves[clamped (_part_params.wave_type.get(), 0u, static_cast<unsigned int> (std::size (_base_waves) - 1))].get();
}


inline DSP::ParametricWave*
Part::modulator_wave() const noexcept
{
	return _modulator_waves[clamped (_part_params.modulator_wave_type.get(), 0u, static_cast<unsigned int> (std::size (_modulator_waves) - 1))].get();
}

} // namespace Yuki

#endif

