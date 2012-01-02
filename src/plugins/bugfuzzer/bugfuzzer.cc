/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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
#include <cstdlib>

// Qt:
#include <QtCore/QTimer>
#include <QtGui/QWidget>
#include <QtGui/QLayout>

// Local:
#include <haruhi/config/all.h>
#include <haruhi/graph/event.h>
#include <haruhi/graph/graph.h>

#include "bugfuzzer.h"


BugFuzzer::BugFuzzer (std::string const& urn, std::string const& title, int id, QWidget* parent):
	Haruhi::Plugin (urn, title, id, parent)
{
	for (int i = 0; i < EventPortsNum; ++i)
		_event_ports[i] = new Haruhi::EventPort (this, QString ("Garbage source %1").arg (i).toStdString(), Haruhi::Port::Output);
	srand (Haruhi::Graph::now());
	QTimer* timer = new QTimer (this);
	QObject::connect (timer, SIGNAL (timeout()), this, SLOT (connect_ports()));
	timer->start (250);
}


BugFuzzer::~BugFuzzer()
{
	for (int i = 0; i < EventPortsNum; ++i)
		delete _event_ports[i];
}


void
BugFuzzer::registered()
{
	enable();
}


void
BugFuzzer::unregistered()
{
}


void
BugFuzzer::process()
{
	sync_inputs();

	// Generate garbage on output ports:
	for (int i = 0; i < EventPortsNum; ++i)
		_event_ports[i]->event_buffer()->push (get_random_event());
}


void
BugFuzzer::connect_ports()
{
	graph()->lock();
	for (int i = 0; i < EventPortsNum; ++i)
	{
		_event_ports[i]->disconnect();
		// Find a unit with event ports:
		for (std::size_t j = 0; j < graph()->units().size(); ++j)
		{
			Haruhi::Graph::Units::const_iterator u = graph()->units().begin();
			std::advance (u, rand() % graph()->units().size());

			for (std::size_t k = 0; k < (*u)->inputs().size(); ++k)
			{
				Haruhi::Ports::iterator p = (*u)->inputs().begin();
				std::advance (p, rand() % (*u)->inputs().size());

				if (dynamic_cast<Haruhi::EventPort*> (*p))
					_event_ports[i]->connect_to (*p);
				goto found;
			}
		}
found:
		;
	}
	graph()->unlock();
}


Haruhi::Event*
BugFuzzer::get_random_event()
{
	switch (rand() % 3)
	{
		case 0:
		{
			int v = rand() % 127;
			int t = rand() % 3;
			Haruhi::VoiceEvent::Type type = t == 0
				? Haruhi::VoiceEvent::Create
				: t == 1
					? Haruhi::VoiceEvent::Release
					: Haruhi::VoiceEvent::Drop;
			float f = Haruhi::VoiceEvent::frequency_from_key_id (v, 440.0);
			return new Haruhi::VoiceEvent (Haruhi::Graph::now(), v, v, type, f, 1.0f * rand() / RAND_MAX);
		}
		case 1:
			return new Haruhi::ControllerEvent (Haruhi::Graph::now(), 1.0f * rand() / RAND_MAX);
		case 2:
			return new Haruhi::VoiceControllerEvent (Haruhi::Graph::now(), rand() % 127, 1.0f * rand() / RAND_MAX);
		default:
			return 0;
	}
}


BugFuzzerFactory::BugFuzzerFactory():
	Haruhi::PluginFactory()
{
}


Haruhi::Plugin*
BugFuzzerFactory::create_plugin (int id, QWidget* parent)
{
	return new BugFuzzer (urn(), title(), id, parent);
}


void
BugFuzzerFactory::destroy_plugin (Haruhi::Plugin* plugin)
{
	delete plugin;
}

