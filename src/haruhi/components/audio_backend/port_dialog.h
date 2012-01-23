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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__PORT_DIALOG_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__PORT_DIALOG_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QDialog>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

namespace AudioBackendImpl {

class Backend;

class PortDialog: public QDialog
{
  public:
	PortDialog (QWidget* parent, Backend* backend);

	virtual ~PortDialog() { }

  protected:
	Backend* _backend;
};

} // namespace AudioBackendImpl

} // namespace Haruhi

#endif

