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

#ifndef HARUHI__PLUGINS__YUKI__OPERATOR_WIDGET_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__OPERATOR_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QSpinBox>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/utility/signal.h>

// Local:
#include "params.h"
#include "part.h"


namespace Yuki {

class OperatorWidget:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

  public:
	OperatorWidget (QWidget* parent, unsigned int operator_no, Params::Operator*, Part*);

	~OperatorWidget();

  private slots:
	/**
	 * Update params from widgets.
	 * Call it when widgets are changed.
	 */
	void
	widgets_to_params();

  private:
	/**
	 * Updates widgets' states from params.
	 * \entry	UI thread only
	 */
	void
	params_to_widgets();

	/**
	 * Call params_to_widgets from UI thread later.
	 * \entry	any thread
	 */
	void
	post_params_to_widgets();

  private:
	Part*				_part;
	Params::Operator*	_params;
	bool				_stop_widgets_to_params;
	bool				_stop_params_to_widgets;

	QSpinBox*			_frequency_numerator;
	QSpinBox*			_frequency_denominator;
	Haruhi::Knob*		_knob_detune;
	QSpinBox*			_octave;
};

} // namespace Yuki

#endif

