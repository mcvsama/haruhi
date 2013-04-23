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

#ifndef HARUHI__PLUGINS__POLYTRAINER__PLUGIN_H__INCLUDED
#define HARUHI__PLUGINS__POLYTRAINER__PLUGIN_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QEvent>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/plugin/plugin.h>
#include <haruhi/graph/event_port.h>

// Local:
#include "piano_widget.h"
#include "staff_widget.h"


namespace Polytrainer {

class Plugin: public Haruhi::Plugin
{
	Q_OBJECT

	class Notify: public QEvent
	{
	  public:
		enum About { Update, Reset };

	  public:
		Notify (About);

		About
		about() const noexcept;

	  private:
		About _about;
	};

  public:
	Plugin (std::string const& urn, std::string const& title, int id, QWidget* parent);

	virtual ~Plugin();

	/*
	 * Plugin implementation.
	 */

	void
	registered() override;

	void
	process() override;

	void
	panic() override;

  private:
	void
	customEvent (QEvent*) override;

  private:
	Haruhi::EventPort*	_input_keyboard;
	Haruhi::EventBuffer	_input_buffer;
	StaffWidget*		_staff_widget;
	PianoWidget*		_piano_widget;
};


inline
Plugin::Notify::Notify (About about):
	QEvent (QEvent::User),
	_about (about)
{ }


inline Plugin::Notify::About
Plugin::Notify::about() const noexcept
{
	return _about;
}

} // namespace Polytrainer

#endif

