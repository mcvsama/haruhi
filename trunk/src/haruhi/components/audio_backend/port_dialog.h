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
 *
 * NOTES
 *
 * Never call JACK functions when session's graph lock is acquired. This may lead to deadlock
 * when JACK will wait for end of its graph processing, and process() function will wait
 * on session's graph lock.
 *
 * As there is assumption that in processing round graph cannot be modified you should only
 * acquire graph lock when changing core objects (ports, unit attributes, connecting ports, etc.).
 */

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__PORT_DIALOG_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__PORT_DIALOG_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QDialog>


namespace Haruhi {

class AudioBackend;

namespace AudioBackendPrivate {

class PortDialog: public QDialog
{
  public:
	PortDialog (QWidget* parent, AudioBackend* backend);

	virtual ~PortDialog() { }

  protected:
	AudioBackend* _backend;
};

} // namespace AudioBackendPrivate

} // namespace Haruhi

#endif

