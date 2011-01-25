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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__OUTPUT_DIALOG_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__OUTPUT_DIALOG_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

// Local:
#include "port_dialog.h"
#include "output_item.h"


namespace Haruhi {

namespace AudioBackendImpl {

class OutputDialog: public PortDialog
{
	Q_OBJECT

  public:
	OutputDialog (QWidget* parent, Backend* backend);

	virtual ~OutputDialog() { }

	void
	from (OutputItem const*);

	void
	apply (OutputItem*) const;

  private slots:
	void
	update_widgets();

	void
	validate_and_accept();

  private:
	OutputItem*		_item;
	QLineEdit*		_name;
	QPushButton*	_accept_button;
	QPushButton*	_reject_button;
};

} // namespace AudioBackendImpl

} // namespace Haruhi

#endif

