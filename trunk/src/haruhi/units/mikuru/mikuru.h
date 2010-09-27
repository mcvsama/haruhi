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

#ifndef HARUHI__UNITS__MIKURU__MIKURU_H__INCLUDED
#define HARUHI__UNITS__MIKURU__MIKURU_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>
#include <set>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QTabWidget>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/config.h>
#include <haruhi/unit_bay.h>
#include <haruhi/presetable.h>
#include <haruhi/core/event.h>
#include <haruhi/core/audio_port.h>
#include <haruhi/core/event_port.h>
#include <haruhi/dsp/smoother.h>
#include <haruhi/unit.h>
#include <haruhi/utility/mutex.h>
#include <haruhi/utility/thread.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "patch.h"
#include "part.h"
#include "voice.h"
#include "synth_thread.h"
#include "general.h"
#include "common_filters.h"
#include "oscillator.h"
#include "double_filter.h"


/**
 * Mikuru virtual-analog synthesizer.
 *
 * Mutexes acquiring order: Core::Graph mutex, _parts_mutex, _synth_threads_mutex.
 */
class Mikuru:
	public Haruhi::Unit,
	public Haruhi::UnitBayAware,
	public Haruhi::Presetable,
	public SaveableState
{
	Q_OBJECT

	friend class MikuruPrivate::Oscillator;
	friend class MikuruPrivate::Filter;
	friend class MikuruPrivate::General;
	friend class MikuruPrivate::Envelopes;
	friend class MikuruPrivate::Part;
	friend class MikuruPrivate::VoiceManager;

  public:
	/**
	 * Notification sent to other Mikurus
	 * when user changes global configuration.
	 */
	class UpdateConfig: public Core::Notification
	{
	  public:
		UpdateConfig (Mikuru* sender, std::string const& urn):
			Core::Notification (urn),
			_sender (sender)
		{ }

		Mikuru*
		sender() const { return _sender; }

	  private:
		Mikuru*	_sender;
	};

	typedef std::list<MikuruPrivate::Part*>  Parts;
	typedef std::set<int>  IDs;

  public:
	Mikuru (Haruhi::UnitFactory*, Haruhi::Session*, std::string const& urn, std::string const& title, int id, QWidget* parent);

	virtual ~Mikuru();

	std::string
	name() const;

	void
	process();

	void
	panic();

	void
	graph_updated();

	Core::EventPort*
	keyboard_port() const { return _port_keyboard; }

	/**
	 * Returns reference to Patch saver/restorer.
	 */
	MikuruPrivate::Patch&
	patch() { return _patch; }

	MikuruPrivate::Patch const&
	patch() const { return _patch; }

	/**
	 * Returns pointer to General.
	 */
	MikuruPrivate::General*
	general() const { return _general; }

	/**
	 * Returns pointer to CommonFilters.
	 */
	MikuruPrivate::CommonFilters*
	common_filters() const { return _common_filters; }

	/**
	 * Returns reference to set of Parts used by synth.
	 */
	Parts&
	parts() { return _parts; }

	Parts const&
	parts() const { return _parts; }

	/**
	 * Returns tab-position of given part.
	 */
	int
	part_tab_position (MikuruPrivate::Part* part) const { return _tabs_widget->indexOf (part); }

	/**
	 * Core::Unit::notify implementation.
	 */
	void
	notify (Core::Notification*);

	/*
	 * Haruhi::UnitBayAware implementation.
	 */

	void
	set_unit_bay (Haruhi::UnitBay*);

	/*
	 * SaveableState implementation.
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

	/*
	 * Presetable implementation.
	 */

	void
	save_preset (QDomElement& element) const { save_state (element); }

	void
	load_preset (QDomElement const& element) { load_state (element); }

	/**
	 * Support for unique IDs for parts, LFOs, ADSRs, etc.
	 */
	IDs&
	ids (std::string const& group) { return _ids_sets[group]; }

	/**
	 * Allocates new ID.
	 */
	int
	allocate_id (std::string const& group);

	/**
	 * Reserves given ID. If not possible (already reserved)
	 * returns new ID.
	 */
	int
	reserve_id (std::string const& group, int id);

	/**
	 * Frees given ID.
	 */
	void
	free_id (std::string const& group, int id);

  public slots:
	void
	update_params();

	void
	update_widgets();

	void
	ensure_there_is_at_least_one_part();

	/**
	 * Creates new part (oscillator, filters, etc).
	 */
	MikuruPrivate::Part*
	add_part();

	/**
	 * Destroys currently selected (as a tab) part.
	 */
	void
	del_part();

	/**
	 * Destroys part by its pointer.
	 */
	void
	del_part (MikuruPrivate::Part*);

	void
	del_all_parts();

  private slots:
	void
	update_ui();

  private:
	/**
	 * Saves global configuration.
	 */
	void
	save_config();

	/**
	 * Loads global configuration.
	 */
	void
	load_config();

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
	process_envelopes();

	void
	process_voice_events();

	void
	process_controller_events();

	void
	process_voices();

	/**
	 * Updates number of synth. threads. Pass 0 to detect automatically.
	 * \entry	Only from UI thread.
	 */
	void
	set_threads_number (int threads);

	/**
	 * Finds least loaded thread for new Voice.
	 * \entry	Any thread.
	 */
	MikuruPrivate::SynthThread*
	select_thread_for_new_voice();

  private:
	MikuruPrivate::SynthThreads		_synth_threads;
	RecursiveMutex					_synth_threads_mutex;
	Parts							_parts;
	RecursiveMutex					_parts_mutex;
	MikuruPrivate::Patch			_patch;

	QCheckBox*						_enabled;
	QLabel*							_current_polyphony;
	QPushButton*					_add_part;
	QPushButton*					_del_part;
	QTimer*							_update_ui_timer;

	MikuruPrivate::General*			_general;
	MikuruPrivate::CommonFilters*	_common_filters;
	QTabWidget*						_tabs_widget;

	DSP::Smoother					_audio_input_smoother_L;
	DSP::Smoother					_audio_input_smoother_R;
	DSP::Smoother					_master_volume_smoother_L;
	DSP::Smoother					_master_volume_smoother_R;

	//
	// Synthesizer ports
	//

	Core::AudioBuffer				_mix_L;
	Core::AudioBuffer				_mix_R;
	Core::AudioBuffer				_filter_buffer_L;
	Core::AudioBuffer				_filter_buffer_R;
	Core::AudioBuffer				_input_buffer_L;
	Core::AudioBuffer				_input_buffer_R;

	// Audio inputs:
	Core::AudioPort*				_audio_input_L;
	Core::AudioPort*				_audio_input_R;

	// Audio outputs:
	Core::AudioPort*				_audio_output_L;
	Core::AudioPort*				_audio_output_R;

	// Drive:
	Core::EventPort*				_port_keyboard;
	Core::EventPort*				_port_sustain;

	//
	// Other
	//

	int								_param_enabled;
	std::map<std::string, IDs>		_ids_sets;
};


class MikuruFactory: public Haruhi::UnitFactory
{
  public:
	MikuruFactory();

	Haruhi::Unit*
	create_unit (Haruhi::Session*, int id, QWidget* parent);

	void
	destroy_unit (Haruhi::Unit* unit);

	InformationMap const&
	information() const;

  private:
	InformationMap _information;
};

#endif

