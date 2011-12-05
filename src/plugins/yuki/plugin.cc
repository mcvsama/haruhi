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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QLayout>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "plugin.h"
#include "part_manager.h"
#include "part_manager_widget.h"


namespace Yuki {

Plugin::Plugin (std::string const& urn, std::string const& title, int id, QWidget* parent):
	Haruhi::Plugin (urn, title, id, parent)
{
	_audio_out[0] = new Haruhi::AudioPort (this, "Ouput 1", Haruhi::Port::Output, 0, Haruhi::Port::StandardAudio);
	_audio_out[1] = new Haruhi::AudioPort (this, "Ouput 2", Haruhi::Port::Output, 0, Haruhi::Port::StandardAudio);
	_voice_in = new Haruhi::EventPort (this, "Voice control", Haruhi::Port::Input, 0, Haruhi::Port::ControlKeyboard);

	_part_manager = new PartManager (this);
	_part_manager_widget = new PartManagerWidget (this, _part_manager);
	_part_manager->set_widget (_part_manager_widget);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (_part_manager_widget);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
}


Plugin::~Plugin()
{
	delete _part_manager_widget;
	delete _part_manager;

	delete _voice_in;
	delete _audio_out[0];
	delete _audio_out[1];
}


void
Plugin::registered()
{
	enable();
}


void
Plugin::unregistered()
{
	panic();
}


void
Plugin::process()
{
	clear_outputs();

	// TODO handle input voice events
}


void
Plugin::panic()
{
}


void
Plugin::graph_updated()
{
	Unit::graph_updated();
}

} // namespace Yuki

