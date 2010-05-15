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
 * As there is assumption that inside of processing round graph cannot be modified you should only
 * acquire graph lock when changing core objects (ports, unit attributes, connecting ports, etc.).
 */

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__INPUT_DIALOG_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__INPUT_DIALOG_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

// Local:
#include "port_dialog.h"
#include "input_item.h"


namespace Haruhi {

namespace AudioBackendPrivate {

class InputDialog: public PortDialog
{
	Q_OBJECT

  public:
	InputDialog (QWidget* parent, AudioBackend* backend);

	virtual ~InputDialog() { }

	void
	from (InputItem const*);

	void
	apply (InputItem*) const;

  private slots:
	void
	update_widgets();

	void
	validate_and_accept();

  private:
	InputItem*		_item;
	QLineEdit*		_name;
	QPushButton*	_accept_button;
	QPushButton*	_reject_button;
};

} // namespace AudioBackendPrivate

} // namespace Haruhi

#endif

