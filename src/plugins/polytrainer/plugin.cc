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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QLayout>
#include <QtGui/QFrame>
#include <QtGui/QApplication>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "plugin.h"
#include "piano_widget.h"


namespace Polytrainer {

Plugin::Plugin (std::string const& urn, std::string const& title, int id, QWidget* parent):
	Haruhi::Plugin (urn, title, id, parent)
{
	_input_keyboard = new Haruhi::EventPort (this, "Voice control", Haruhi::Port::Input);

	_staff_widget = new StaffWidget (this);

	QFrame* piano_widget_frame = new QFrame (this);
	piano_widget_frame->setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);
	QVBoxLayout* piano_frame_layout = new QVBoxLayout (piano_widget_frame);
	piano_frame_layout->setMargin (0);
	piano_frame_layout->addWidget (_staff_widget);

	_piano_widget = new PianoWidget (this);
	_piano_widget->set_key_range (Haruhi::Key (21), Haruhi::Key (108));

	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (piano_widget_frame);
	layout->addWidget (_piano_widget);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	//XXX
	_staff_widget->set_note (Haruhi::Key (52));
}


Plugin::~Plugin()
{
	delete _input_keyboard;
}


void
Plugin::registered()
{
	enable();
}


void
Plugin::process()
{
	sync_inputs();

	_input_buffer.mixin (_input_keyboard->event_buffer());

	QApplication::postEvent (this, new Notify (Notify::Update));
}


void
Plugin::panic()
{
	QApplication::postEvent (this, new Notify (Notify::Reset));
}


void
Plugin::customEvent (QEvent* event)
{
	Notify* notify = dynamic_cast<Notify*> (event);
	if (notify)
	{
		switch (notify->about())
		{
			case Notify::Update:
				for (auto e: _input_buffer.events())
				{
					if (e->event_type() == Haruhi::Event::VoiceEventType)
					{
						Haruhi::VoiceEvent const* ev = static_cast<Haruhi::VoiceEvent const*> (e.get());
						if (ev->action() == Haruhi::VoiceEvent::Action::Create)
						{
							_piano_widget->set_key_depressed (ev->key(), true);
							_staff_widget->set_note (ev->key());
						}
						else if (ev->action() == Haruhi::VoiceEvent::Action::Drop)
							_piano_widget->set_key_depressed (ev->key(), false);
					}
				}
				_input_buffer.clear();
				break;

			case Notify::Reset:
				_piano_widget->reset_all_depressions();
				break;
		}
	}
}

} // namespace Polytrainer

