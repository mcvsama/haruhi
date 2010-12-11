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

#ifndef HARUHI__SETTINGS__DEVICES_MANAGER_SETTINGS_H__INCLUDED
#define HARUHI__SETTINGS__DEVICES_MANAGER_SETTINGS_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtXml/QDomNode>

// Haruhi:
#include <haruhi/utility/saveable_state.h>

// Local:
#include "settings.h"


namespace Haruhi {

class DevicesManagerSettings: public Settings::Module
{
  public:
	class Device
	{
	  public:
		Device (QString const& name, QDomElement element):
			_name (name),
			_element (element.cloneNode (true).toElement())
		{ }

		QString
		name() const { return _name; }

		QDomElement&
		element() { return _element; }

		QDomElement const&
		element() const { return _element; }

	  private:
		QString		_name;
		QDomElement	_element;
	};

	typedef std::list<Device> Devices;

  public:
	DevicesManagerSettings();

	void
	save_state (QDomElement& element) const;

	void
	load_state (QDomElement const& element);

	Devices&
	devices() { return _devices; }

	Devices const&
	devices() const { return _devices; }

	/**
	 * Creates new Device and uses @device to store
	 * information in it.
	 */
	void
	save_device (QString const& name, SaveableState const& device);

  private:
	Devices _devices;
};

} // namespace Haruhi

#endif

