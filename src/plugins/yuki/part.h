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
#include <haruhi/dsp/wavetable.h>
#include <haruhi/dsp/parametric_wave.h>
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
		UpdateWavetableWorkUnit (Part*);

		/**
		 * Prepare work unit for another work.
		 * \param	base_wave Base wave for computation. It's cloned inside.
		 * \param	modulator_wave Modulator wave. It's cloned.
		 * \param	wavetable Target wavetable object.
		 * \param	serial Update request ID.
		 */
		void
		reset (DSP::Wavetable* wavetable, unsigned int serial);

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
		Part*					_part;
		DSP::Wavetable*			_wavetable;
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
			VoiceParamUpdater (VoiceManager* voice_manager, ParamPtr param_ptr);

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
			FilterParamUpdater (VoiceManager* voice_manager, unsigned int filter_no, ParamPtr param_ptr);

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
		typedef std::vector<std::vector<Haruhi::EventPort*> > MatrixPorts;

	  public:
		PartPorts (Plugin*, unsigned int part_id);

		~PartPorts();

	  public:
		// Waveform ports:
		Haruhi::EventPort* wave_shape;
		Haruhi::EventPort* modulator_amplitude;
		Haruhi::EventPort* modulator_index;
		Haruhi::EventPort* modulator_shape;

		// Part ports:
		Haruhi::EventPort* volume;
		Haruhi::EventPort* portamento_time;
		Haruhi::EventPort* phase;

		// Polyphonic-input ports:
		Haruhi::EventPort* amplitude;
		Haruhi::EventPort* frequency;
		Haruhi::EventPort* panorama;
		Haruhi::EventPort* detune;
		Haruhi::EventPort* pitchbend;
		Haruhi::EventPort* velocity_sens;
		Haruhi::EventPort* unison_index;
		Haruhi::EventPort* unison_spread;
		Haruhi::EventPort* unison_init;
		Haruhi::EventPort* unison_noise;
		Haruhi::EventPort* unison_vibrato_level;
		Haruhi::EventPort* unison_vibrato_frequency;
		Haruhi::EventPort* noise_level;

		// Filter ports:
		Haruhi::EventPort* filter_frequency[Params::Voice::FiltersNumber];
		Haruhi::EventPort* filter_resonance[Params::Voice::FiltersNumber];
		Haruhi::EventPort* filter_gain[Params::Voice::FiltersNumber];
		Haruhi::EventPort* filter_attenuation[Params::Voice::FiltersNumber];

		// Operator ports:
		Haruhi::EventPort* operator_detune[Params::Part::OperatorsNumber];

		// Modulator matrix:
		MatrixPorts	fm_matrix;
		MatrixPorts	am_matrix;

		// Port group:
		Haruhi::PortGroup* port_group;
	};

	/**
	 * Contains ControllerProxies that connect event ports and params.
	 */
	class PartControllerProxies: private Noncopyable
	{
	  public:
		typedef std::vector<std::vector<Haruhi::ControllerProxy*> > MatrixControllerProxies;

	  public:
		PartControllerProxies (PartManager*, PartPorts*, Params::Part*);

		~PartControllerProxies();

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
		Haruhi::ControllerProxy volume;
		Haruhi::ControllerProxy portamento_time;
		Haruhi::ControllerProxy phase;
		Haruhi::ControllerProxy noise_level;
		Haruhi::ControllerProxy wave_shape;
		Haruhi::ControllerProxy modulator_amplitude;
		Haruhi::ControllerProxy modulator_index;
		Haruhi::ControllerProxy modulator_shape;

		// Voice:
		Haruhi::ControllerProxy amplitude;
		Haruhi::ControllerProxy frequency;
		Haruhi::ControllerProxy panorama;
		Haruhi::ControllerProxy detune;
		Haruhi::ControllerProxy pitchbend;
		Haruhi::ControllerProxy velocity_sens;
		Haruhi::ControllerProxy unison_index;
		Haruhi::ControllerProxy unison_spread;
		Haruhi::ControllerProxy unison_init;
		Haruhi::ControllerProxy unison_noise;
		Haruhi::ControllerProxy unison_vibrato_level;
		Haruhi::ControllerProxy unison_vibrato_frequency;

		// Filters:
		Haruhi::ControllerProxy* filter_frequency[Params::Voice::FiltersNumber];
		Haruhi::ControllerProxy* filter_resonance[Params::Voice::FiltersNumber];
		Haruhi::ControllerProxy* filter_gain[Params::Voice::FiltersNumber];
		Haruhi::ControllerProxy* filter_attenuation[Params::Voice::FiltersNumber];

		// Operators:
		Haruhi::ControllerProxy* operator_detune[Params::Part::OperatorsNumber];

		// Modulator matrix:
		MatrixControllerProxies fm_matrix;
		MatrixControllerProxies am_matrix;

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

		~ParamUpdaters();

	  public:
		// Voice params updaters:
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	amplitude;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	frequency;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	panorama;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	detune;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	pitchbend;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	velocity_sens;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	unison_index;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	unison_spread;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	unison_init;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	unison_noise;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	unison_vibrato_level;
		VoiceParamUpdater<Params::Voice::ControllerParamPtr>	unison_vibrato_frequency;

		// Filter params updaters:
		FilterParamUpdater<Params::Filter::ControllerParamPtr>*	filter_frequency[Params::Voice::FiltersNumber];
		FilterParamUpdater<Params::Filter::ControllerParamPtr>*	filter_resonance[Params::Voice::FiltersNumber];
		FilterParamUpdater<Params::Filter::ControllerParamPtr>*	filter_gain[Params::Voice::FiltersNumber];
		FilterParamUpdater<Params::Filter::ControllerParamPtr>*	filter_attenuation[Params::Voice::FiltersNumber];
		FilterParamUpdater<Params::Filter::IntParamPtr>*		filter_enabled[Params::Voice::FiltersNumber];
		FilterParamUpdater<Params::Filter::IntParamPtr>*		filter_type[Params::Voice::FiltersNumber];
		FilterParamUpdater<Params::Filter::IntParamPtr>*		filter_stages[Params::Voice::FiltersNumber];
		FilterParamUpdater<Params::Filter::IntParamPtr>*		filter_limiter_enabled[Params::Voice::FiltersNumber];
	};

  public:
	Part (PartManager*, WorkPerformer* rendering_work_performer, Params::Main* main_params, unsigned int id);

	~Part();

	/**
	 * Return PartManager that manages this Part.
	 */
	PartManager*
	part_manager() const;

	/**
	 * Return part params.
	 */
	Params::Part*
	part_params();

	/**
	 * Return ports list object.
	 */
	PartPorts*
	ports();

	/**
	 * Return proxies list created by the part.
	 * Needed for the widget to link Knobs to proxies.
	 */
	PartControllerProxies*
	proxies();

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

	/**
	 * Return currently selected base wave object.
	 */
	DSP::ParametricWave*
	base_wave() const;

	/**
	 * Return currently selected modulator wave object.
	 */
	DSP::ParametricWave*
	modulator_wave() const;

	/**
	 * Allocate Wave* that incorporates harmonics and modulation
	 * taken from parameters object at the time of the call.
	 * Must be deleted after use.
	 * \threadsafe
	 */
	DSP::Wave*
	final_wave() const;

	/*
	 * SaveableState implementation
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

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
	PartManager*				_part_manager;
	VoiceManager*				_voice_manager;
	Params::Part				_part_params;
	DSP::ParametricWave*		_base_waves[9];
	DSP::ParametricWave*		_modulator_waves[4];
	DSP::Wavetable*				_wavetables[2];
	Atomic<bool>				_switch_wavetables;
	Atomic<unsigned int>		_wt_update_request;
	Atomic<unsigned int>		_wt_serial;
	UpdateWavetableWorkUnit*	_wt_wu;
	bool						_wt_wu_ever_started;
	PartPorts					_ports;
	PartControllerProxies		_proxies;
	ParamUpdaters				_updaters;
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


template<class ParamPtr>
	inline
	Part::VoiceParamUpdater<ParamPtr>::VoiceParamUpdater (VoiceManager* voice_manager, ParamPtr param_ptr):
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
	Part::FilterParamUpdater<ParamPtr>::FilterParamUpdater (VoiceManager* voice_manager, unsigned int filter_no, ParamPtr param_ptr):
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
Part::part_manager() const
{
	return _part_manager;
}


inline Params::Part*
Part::part_params()
{
	return &_part_params;
}


inline Part::PartPorts*
Part::ports()
{
	return &_ports;
}


inline Part::PartControllerProxies*
Part::proxies()
{
	return &_proxies;
}


inline DSP::ParametricWave*
Part::base_wave() const
{
	return _base_waves[bound (_part_params.wave_type.get(), 0u, static_cast<unsigned int> (countof (_base_waves) - 1))];
}


inline DSP::ParametricWave*
Part::modulator_wave() const
{
	return _modulator_waves[bound (_part_params.modulator_wave_type.get(), 0u, static_cast<unsigned int> (countof (_modulator_waves) - 1))];
}

} // namespace Yuki

#endif

