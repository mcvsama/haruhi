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

#ifndef HARUHI__COMPONENTS__PORTS_CONNECTOR__CREATE_PORT_DIALOG_H__INCLUDED
#define HARUHI__COMPONENTS__PORTS_CONNECTOR__CREATE_PORT_DIALOG_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QRadioButton>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QDialog>


namespace Haruhi {

namespace PortsConnectorPrivate {

class CreatePortDialog: public QDialog
{
	Q_OBJECT

  public:
	enum Graph { Audio, Event };

  public:
	CreatePortDialog (QWidget* parent);

	Graph
	port_type() const;

	QString
	port_name() const;

  private slots:
	void
	state_changed();

	void
	validate_and_accept();

  private:
	QRadioButton*	_button_audio;
	QRadioButton*	_button_event;
	QPushButton*	_accept_button;
	QPushButton*	_reject_button;
	QVBoxLayout*	_layout;
	QLineEdit*		_port_name;
	static Graph	_last_selected_port_type;
};

} // namespace PortsConnectorPrivate

} // namespace Haruhi

#endif

