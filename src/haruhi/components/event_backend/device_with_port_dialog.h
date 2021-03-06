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

#ifndef HARUHI__COMPONENTS__EVENT_BACKEND__DEVICE_WITH_PORT_DIALOG_H__INCLUDED
#define HARUHI__COMPONENTS__EVENT_BACKEND__DEVICE_WITH_PORT_DIALOG_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/components/devices_manager/device_dialog.h>


namespace Haruhi {

namespace EventBackendImpl {

using DevicesManager::DeviceItem;

class DeviceWithPortDialog: public DevicesManager::DeviceDialog
{
  public:
	DeviceWithPortDialog (QWidget* parent);

	void
	apply (DeviceItem*) const override;
};

} // namespace EventBackendImpl

} // namespace Haruhi

#endif

