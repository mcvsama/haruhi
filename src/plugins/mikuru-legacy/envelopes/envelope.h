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

#ifndef HARUHI__PLUGINS__MIKURU__ENVELOPES__ENVELOPE_H__INCLUDED
#define HARUHI__PLUGINS__MIKURU__ENVELOPES__ENVELOPE_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Qt:
#include <QtGui/QWidget>


namespace MikuruPrivate {

class Part;
class Voice;
class VoiceManager;

class Envelope: public QWidget
{
  public:
	Envelope (QWidget* parent);

	virtual ~Envelope() { }

	virtual int
	id() const = 0;

	virtual void
	voice_created (VoiceManager*, Voice*) { }

	virtual void
	voice_released (VoiceManager*, Voice*) { }

	virtual void
	voice_dropped (VoiceManager*, Voice*) { }

	virtual void
	new_part (Part* part) { }

	virtual void
	process() = 0;

	virtual void
	resize_buffers (std::size_t size) = 0;
};

} // namespace MikuruPrivate

#endif

