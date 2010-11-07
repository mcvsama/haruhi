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

#ifndef HARUHI__UNITS__MIKURU__ENVELOPES_H__INCLUDED
#define HARUHI__UNITS__MIKURU__ENVELOPES_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>
#include <map>

// Qt:
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QStackedWidget>

// Haruhi:
#include <haruhi/haruhi.h>
#include <haruhi/utility/mutex.h>

// Local:
#include "widgets.h"
#include "params.h"


class Mikuru;

namespace MikuruPrivate {

class Voice;
class VoiceManager;
class Part;
class ADSR;
class EG;
class LFO;


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


class Envelopes: public QWidget
{
	Q_OBJECT

  public:
	typedef std::list<Envelope*> EnvelopesList;

  public:
	Envelopes (Mikuru* mikuru, QWidget* parent);

	~Envelopes();

	EnvelopesList&
	envelopes() { return _envelopes; }

	void
	process();

	void
	resize_buffers (std::size_t size);

	void
	notify_voice_created (VoiceManager*, Voice*);

	void
	notify_voice_released (VoiceManager*, Voice*);

	void
	notify_voice_dropped (VoiceManager*, Voice*);

	void
	notify_new_part (Part* part);

	/**
	 * Returns tab-poisition of given envelope.
	 */
	int
	envelope_tab_position (MikuruPrivate::Envelope* envelope) const { return _tabs->indexOf (envelope); }

  private:
	void
	update_widgets();

  public slots:
	ADSR*
	add_adsr (int id = 0);

	EG*
	add_eg (int id = 0);

	LFO*
	add_lfo (int id = 0);

	void
	destroy_envelope();

	void
	destroy_envelope (Envelope*);

	void
	destroy_all_envelopes();

	void
	show_first();

  private:
	Mikuru*			_mikuru;
	EnvelopesList	_envelopes;
	RecursiveMutex	_envelopes_mutex;

	QPushButton*	_add_adsr;
	QPushButton*	_add_envelope;
	QPushButton*	_add_lfo;
	QPushButton*	_remove_envelope;
	QTabWidget*		_tabs;
	QStackedWidget*	_stack;
	QLabel*			_placeholder;
};

} // namespace MikuruPrivate

#endif

