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

// Local:
#include "port_dialog.h"


namespace Haruhi {

namespace AudioBackendImpl {

PortDialog::PortDialog (QWidget* parent, Backend* backend):
	QDialog (parent),
	_backend (backend)
{
}

} // namespace AudioBackendImpl

} // namespace Haruhi

