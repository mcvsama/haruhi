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
 *
 * TODO Support for "adjust" in percents
 */

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QGridLayout>
#include <QtGui/QToolTip>
#include <QtGui/QGroupBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "../mikuru.h"
#include "delay.h"


namespace MikuruPrivate {

DelayModel::DelayModel():
	_tempo (120.0f),
	_1_div_note_length (4),
	_note_multiplicator (1),
	_feedback (0.0f),
	_level (1.0f),
	_sample_rate (48000),
	_buffer_size (64)
{
	update();
}


void
DelayModel::set_tempo (float tempo)
{
	limit_value (tempo, MinimumTempo, MaximumTempo);
	if (_tempo != tempo)
	{
		_tempo = tempo;
		update();
	}
}


void
DelayModel::set_note_length (int p1_div_note_length)
{
	if (_1_div_note_length != p1_div_note_length)
	{
		_1_div_note_length = p1_div_note_length;
		update();
	}
}


void
DelayModel::set_note_multiplicator (int note_multiplicator)
{
	if (_note_multiplicator != note_multiplicator)
	{
		_note_multiplicator = note_multiplicator;
		update();
	}
}


void
DelayModel::set_sample_rate (unsigned int sample_rate)
{
	if (_sample_rate != sample_rate)
	{
		_sample_rate = sample_rate;
		update();
	}

	update_delay_buffer();
}


void
DelayModel::set_buffer_size (std::size_t buffer_size)
{
	if (_buffer_size != buffer_size)
	{
		_buffer_size = buffer_size;
		update();
	}

	update_delay_buffer();
}


void
DelayModel::get_cross_feedback (Haruhi::AudioBuffer* result)
{
	_delay_line.read (result->begin());
	result->attenuate (_cross_feedback);
}


void
DelayModel::process (Haruhi::AudioBuffer* input, Haruhi::AudioBuffer* output)
{
	assert (input->size() == output->size());
	assert (input->size() == _delay_line.size());

	if (!_enabled)
		output->clear();
	else
	{
		// Feedback, damping and mixing with input:
		_delay_line.read (output->begin());
		output->attenuate (_feedback);
		output->mixin (input);

		// Get result:
		_delay_line.write (output->begin());
		_delay_line.read (output->begin());
		output->attenuate (_level);
	}
}


void
DelayModel::panic()
{
	_delay_line.clear();
}


void
DelayModel::update()
{
	// Ensure times is <= 1_div_note_length to prevent buffer overruns:
	int times = bound (_note_multiplicator, 1, _1_div_note_length);
	_delay_line.set_delay (times * (1.0f / _1_div_note_length) * 4.0f * (60.0f / _tempo) * _sample_rate);
	_delay_line.set_size (_buffer_size);
}


void
DelayModel::update_delay_buffer()
{
	// Max note * 4 quarter notes * min expected tempo * current sample rate (+2 is for float<->int conversion safety):
	std::size_t max_delay = std::max (static_cast<unsigned int> (4.0f * (60.0f / MinimumTempo) * _sample_rate + 2), _buffer_size);
	if (_delay_line.max_delay() != max_delay)
		_delay_line.set_max_delay (max_delay);
}


ChannelPanel::ChannelPanel (QWidget* parent, Delay* delay, QString const& label_text, Haruhi::ControllerParam* param_feedback, Haruhi::ControllerParam* param_cross_feedback, Haruhi::ControllerParam* param_level):
	QWidget (parent),
	_delay (delay)
{
	_enabled = new QPushButton (label_text, this);
	_enabled->setCheckable (true);
	QObject::connect (_enabled, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	_note = new QComboBox (this);
	// TODO dotted notes
	_note->addItem (Resources::NoteIcons::note_01(), "1/1", QVariant::fromValue (1));
	_note->addItem (Resources::NoteIcons::note_02(), "1/2", QVariant::fromValue (2));
	_note->addItem (Resources::NoteIcons::note_04(), "1/4", QVariant::fromValue (4));
	_note->addItem (Resources::NoteIcons::note_08(), "1/8", QVariant::fromValue (8));
	_note->addItem (Resources::NoteIcons::note_16(), "1/16", QVariant::fromValue (16));
	_note->addItem (Resources::NoteIcons::note_32(), "1/32", QVariant::fromValue (32));
	QObject::connect (_note, SIGNAL (activated (int)), this, SLOT (update_params()));
	QObject::connect (_note, SIGNAL (activated (int)), this, SLOT (update_widgets()));

	_times = new QSpinBox (this);
	_times->setPrefix ("× ");
	_times->setSuffix (" times");
	_times->setRange (1, 32);
	QToolTip::add (_times, "Note × times");
	QObject::connect (_times, SIGNAL (valueChanged (int)), this, SLOT (update_params()));

	_adjust = new QDoubleSpinBox (this);
	_adjust->setDecimals (2);
	_adjust->setSingleStep (0.01);
	_adjust->setPrefix ("adjust ");
	_adjust->setSuffix ("%");
	_adjust->setRange (-2.50f, +2.50f);
	QToolTip::add (_adjust, "Delay adjustment");
	QObject::connect (_adjust, SIGNAL (valueChanged (double)), this, SLOT (update_params()));

	_knob_feedback = new Haruhi::Knob (this, 0, param_feedback, "Feedback", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Delay::Feedback, 100), 2);
	_knob_cross_feedback = new Haruhi::Knob (this, 0, param_cross_feedback, "Cross Fb", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Delay::CrossFeedback, 100), 2);
	_knob_level = new Haruhi::Knob (this, 0, param_level, "Level", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Delay::Level, 100), 2);

	_knob_feedback->set_unit_bay (_delay->_mikuru->unit_bay());
	_knob_cross_feedback->set_unit_bay (_delay->_mikuru->unit_bay());
	_knob_level->set_unit_bay (_delay->_mikuru->unit_bay());

	QHBoxLayout* knobs_layout = new QHBoxLayout();
	knobs_layout->setSpacing (Config::Spacing);
	knobs_layout->setMargin (0);
	knobs_layout->addWidget (_knob_feedback);
	knobs_layout->addWidget (_knob_cross_feedback);
	knobs_layout->addWidget (_knob_level);

	QGridLayout* layout = new QGridLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (_enabled, 0, 0, 1, 1);
	layout->addWidget (_note, 0, 1, 1, 1);
	layout->addWidget (_times, 1, 0);
	layout->addWidget (_adjust, 1, 1);
	layout->addLayout (knobs_layout, 2, 0, 1, 2);

	update_widgets();
}


void
ChannelPanel::update_widgets()
{
	_times->setMaximum (_note->itemData (_note->currentIndex()).value<int>());
}


void
ChannelPanel::update_params()
{
	emit params_changed();
}


Delay::Delay (int id, Mikuru* mikuru, QWidget* parent):
	Effect (id, "delays", mikuru, "Delay", new Params::Delay(), parent),
	_mikuru (mikuru),
	_params (static_cast<Params::Delay*> (Effect::params())),
	_loading_params (false)
{
	_channel[L] = new ChannelPanel (this, this, "Enable left", &_params->feedback_l, &_params->cross_feedback_l, &_params->level_l);
	_channel[R] = new ChannelPanel (this, this, "Enable right", &_params->feedback_r, &_params->cross_feedback_r, &_params->level_r);

	for (int i = 0; i < NumChannels; ++i)
		QObject::connect (_channel[i], SIGNAL (params_changed()), this, SLOT (update_params()));

	_tempo_spinbox = new QDoubleSpinBox (this);
	_tempo_spinbox->setRange (20.0, 400.0);
	_tempo_spinbox->setDecimals (2);
	_tempo_spinbox->setWrapping (true);
	_tempo_spinbox->setValue (120.0);
	QObject::connect (_tempo_spinbox, SIGNAL (valueChanged (double)), this, SLOT (update_params()));

	_use_graph_tempo = new QCheckBox ("Use master");
	QObject::connect (_use_graph_tempo, SIGNAL (toggled (bool)), this, SLOT (update_params()));
	QObject::connect (_use_graph_tempo, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	_tempo_label = new QLabel ("Tempo:", this);

	QVBoxLayout* tempo_layout = new QVBoxLayout();
	tempo_layout->setMargin (0);
	tempo_layout->setSpacing (Config::Spacing);
	tempo_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
	tempo_layout->addWidget (_tempo_label);
	tempo_layout->addWidget (_tempo_spinbox);
	tempo_layout->addWidget (_use_graph_tempo);
	tempo_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	QGroupBox* group_box = new QGroupBox (this);

	QHBoxLayout* group_layout = new QHBoxLayout (group_box);
	group_layout->setMargin (Config::Margin);
	group_layout->setSpacing (Config::Spacing);
	group_layout->addWidget (_channel[L]);
	group_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	group_layout->addLayout (tempo_layout);
	group_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	group_layout->addWidget (_channel[R]);

	QGridLayout* layout = new QGridLayout (parent_widget());
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (group_box, 0, 0);

	load_params();
	graph_updated();
	update_widgets();
}


Delay::~Delay()
{
	delete _channel[L];
	delete _channel[R];
}


void
Delay::graph_updated()
{
	std::size_t bs = _mikuru->graph()->buffer_size();

	_tmp_1.resize (bs);
	_tmp_2.resize (bs);

	for (int i = 0; i < NumChannels; ++i)
	{
		_channel[i]->delay_model().set_buffer_size (bs);
		_channel[i]->delay_model().set_sample_rate (_mikuru->graph()->sample_rate());
	}
}


void
Delay::process_events()
{
	// TODO
}


void
Delay::process (Haruhi::AudioBuffer* in1, Haruhi::AudioBuffer* in2, Haruhi::AudioBuffer* out1, Haruhi::AudioBuffer* out2)
{
	update_channels_params();

	// Process cross-feedback:
	_channel[L]->delay_model().get_cross_feedback (&_tmp_2);
	_channel[R]->delay_model().get_cross_feedback (&_tmp_1);
	_tmp_1.mixin (in1);
	_tmp_2.mixin (in2);

	// Process delays:
	_channel[L]->delay_model().process (&_tmp_1, out1);
	_channel[R]->delay_model().process (&_tmp_2, out2);
}


void
Delay::panic()
{
	for (int i = 0; i < NumChannels; ++i)
		_channel[i]->delay_model().panic();
	_tmp_1.clear();
	_tmp_2.clear();
}


void
Delay::load_params()
{
	// Copy params:
	Params::Delay p (*_params);
	_loading_params = true;

	Effect::load_params();

	_tempo_spinbox->setValue (p.tempo.get() / 1000.0f);
	_use_graph_tempo->setChecked (p.tempo.get() == 0);
	_channel[L]->enabled()->setChecked (p.enabled_l);
	_channel[R]->enabled()->setChecked (p.enabled_r);
	_channel[L]->note()->setCurrentIndex (_channel[L]->note()->findData (QVariant::fromValue (p.note_length_l.get())));
	_channel[R]->note()->setCurrentIndex (_channel[R]->note()->findData (QVariant::fromValue (p.note_length_r.get())));
	_channel[L]->times()->setValue (p.note_multiplicator_l);
	_channel[R]->times()->setValue (p.note_multiplicator_r);

	_loading_params = false;
}


void
Delay::load_params (Params::Delay& params)
{
	*_params = params;
	load_params();
}


void
Delay::update_params()
{
	if (_loading_params)
		return;

	Effect::update_params();

	int tempo = 1000.0f * _tempo_spinbox->value();
	if (_use_graph_tempo->isChecked())
		tempo = 0;

	_params->tempo = tempo;
	_params->enabled_l = _channel[L]->enabled()->isChecked();
	_params->enabled_r = _channel[R]->enabled()->isChecked();
	_params->note_length_l = _channel[L]->note()->itemData (_channel[L]->note()->currentIndex()).value<int>();
	_params->note_length_r = _channel[R]->note()->itemData (_channel[R]->note()->currentIndex()).value<int>();
	_params->note_multiplicator_l = _channel[L]->times()->value();
	_params->note_multiplicator_r = _channel[R]->times()->value();

	// Knob params are updated automatically using #assign_parameter.

	_params->sanitize();
}


void
Delay::update_channels_params()
{
	float tempo = _params->tempo / 1000.0f;
	if (_params->tempo.get() == 0)
		tempo = _mikuru->graph()->tempo();

	_channel[L]->delay_model().set_enabled (_params->enabled_l);
	_channel[R]->delay_model().set_enabled (_params->enabled_r);
	_channel[L]->delay_model().set_tempo (tempo);
	_channel[R]->delay_model().set_tempo (tempo);
	_channel[L]->delay_model().set_note_length (_params->note_length_l);
	_channel[R]->delay_model().set_note_length (_params->note_length_r);
	_channel[L]->delay_model().set_note_multiplicator (_params->note_multiplicator_l);
	_channel[R]->delay_model().set_note_multiplicator (_params->note_multiplicator_r);
	_channel[L]->delay_model().set_feedback (_params->feedback_l.to_f());
	_channel[R]->delay_model().set_feedback (_params->feedback_r.to_f());
	_channel[L]->delay_model().set_cross_feedback (_params->cross_feedback_l.to_f());
	_channel[R]->delay_model().set_cross_feedback (_params->cross_feedback_r.to_f());
	_channel[L]->delay_model().set_level (_params->level_l.to_f());
	_channel[R]->delay_model().set_level (_params->level_r.to_f());
}


void
Delay::update_widgets()
{
	bool use_graph_tempo = _use_graph_tempo->isChecked();
	_tempo_label->setEnabled (!use_graph_tempo);
	_tempo_spinbox->setEnabled (!use_graph_tempo);
	_channel[L]->update_widgets();
	_channel[R]->update_widgets();
}

} // namespace MikuruPrivate

