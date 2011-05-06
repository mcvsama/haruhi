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

#ifndef HARUHI__UNITS__MIKURU__EFFECTS__DELAY_H__INCLUDED
#define HARUHI__UNITS__MIKURU__EFFECTS__DELAY_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/audio_buffer.h>
#include <haruhi/dsp/delay_line.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "effect.h"


class Mikuru;

namespace MikuruPrivate {

class Delay;

// Need to put it here, since ChannelPanel uses it, and
// ChannelPanel must be outside of Delay class, since Qt sucks.
// See below.
class DelayModel
{
	static const float MinimumTempo = 20.0f;
	static const float MaximumTempo = 400.0f;

  public:
	DelayModel();

	void
	set_enabled (bool enabled) { _enabled = enabled; }

	/**
	 * Sets tempo in quarter notes per minute (BPM).
	 */
	void
	set_tempo (float tempo);

	/**
	 * Sets base note: 1/note_length (1/1, 1/2, 1/4, etc).
	 */
	void
	set_note_length (int p1_div_note_length);

	/**
	 * Sets note multiplicator.
	 */
	void
	set_note_multiplicator (int note_multiplicator);

	/**
	 * Sets feedback level.
	 * Argument: [0..1].
	 */
	void
	set_feedback (float feedback) { _feedback = feedback; }

	/**
	 * Sets cross-feedback level.
	 * Argument: [0..1].
	 */
	void
	set_cross_feedback (float cross_feedback) { _cross_feedback = cross_feedback; }

	/**
	 * Sets output level.
	 * Argument: [0..1].
	 */
	void
	set_level (float level) { _level = level; }

	/**
	 * Set current sample rate.
	 */
	void
	set_sample_rate (unsigned int sample_rate);

	/**
	 * Set current graph's buffer size.
	 */
	void
	set_buffer_size (std::size_t buffer_size);

	/**
	 * Puts attenuated cross-feedback to given buffer.
	 * Should be called before process.
	 */
	void
	get_cross_feedback (Haruhi::AudioBuffer* result);

	/**
	 * Processes data.
	 */
	void
	process (Haruhi::AudioBuffer* input, Haruhi::AudioBuffer* output);

	/**
	 * Stops echo effect instantly.
	 */
	void
	panic();

  private:
	/**
	 * Recalculates internal values and resizes
	 * buffers.
	 */
	void
	update();

	/**
	 * Sets buffer size for delay line.
	 * Should not be called from within processing round,
	 * rather only when Graph changes its parameters.
	 */
	void
	update_delay_buffer();

  private:
	bool			_enabled;
	float			_tempo;
	int				_1_div_note_length;
	int				_note_multiplicator;
	float			_feedback;
	float			_cross_feedback;
	float			_level;
	unsigned int	_sample_rate;
	std::size_t		_buffer_size;
	DSP::DelayLine	_delay_line;
};


// FUCK! Can't be a nested class due to Qt's MOC limitations,
// which are not fixed yet!
class ChannelPanel: public QWidget
{
	Q_OBJECT

  public:
	ChannelPanel (QWidget* parent, Delay* delay, QString const& label, Haruhi::ControllerParam* param_feedback, Haruhi::ControllerParam* param_cross_feedback, Haruhi::ControllerParam* param_level);

	DelayModel&
	delay_model() { return _delay_model; }

	QPushButton*
	enabled() const { return _enabled; }

	QComboBox*
	note() const { return _note; }

	QSpinBox*
	times() const { return _times; }

	QDoubleSpinBox*
	adjust() const { return _adjust; }

	Haruhi::Knob*
	knob_feedback() const { return _knob_feedback; }

	Haruhi::Knob*
	knob_cross_feedback() const { return _knob_cross_feedback; }

	Haruhi::Knob*
	knob_level() const { return _knob_level; }

  public slots:
	void
	update_widgets();

  signals:
	/**
	 * Sent when user manipulates non-controller widgets.
	 */
	void
	params_changed();

  private slots:
	/**
	 * Updates model from widgets.
	 */
	void
	update_params();

  private:
	Delay*				_delay;
	DelayModel			_delay_model;

	QPushButton*		_enabled;
	QComboBox*			_note;
	QSpinBox*			_times;
	QDoubleSpinBox*		_adjust;

	Haruhi::Knob*		_knob_feedback;
	Haruhi::Knob*		_knob_cross_feedback;
	Haruhi::Knob*		_knob_level;
};


class Delay: public Effect
{
	Q_OBJECT

	friend class Patch;
	friend class ChannelPanel;

	static const int L = 0;
	static const int R = 1;
	static const int NumChannels = 2;

  public:
	Delay (int id, Mikuru* mikuru, QWidget* parent);

	~Delay();

	Params::Delay*
	params() const { return _params; }

	void
	graph_updated();

	void
	process_events();

	void
	process (Haruhi::AudioBuffer* in1, Haruhi::AudioBuffer* in2, Haruhi::AudioBuffer* out1, Haruhi::AudioBuffer* out2);

	void
	panic();

	ChannelPanel**
	channel_panels() { return _channel; }

  public slots:
	/**
	 * Loads widgets values from Params struct.
	 */
	void
	load_params();

	void
	load_params (Params::Delay& params);

	/**
	 * Updates Params structure from widgets.
	 */
	void
	update_params();

  private:
	void
	update_channels_params();

  private slots:
	void
	update_widgets();

  private:
	Mikuru*				_mikuru;
	Params::Delay*		_params;
	bool				_loading_params;

	QLabel*				_tempo_label;
	QDoubleSpinBox*		_tempo_spinbox;
	QCheckBox*			_use_graph_tempo;
	Haruhi::AudioBuffer	_tmp_1;
	Haruhi::AudioBuffer	_tmp_2;
	ChannelPanel*		_channel[2];
};

} // namespace MikuruPrivate

#endif

