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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>

// Lib:
#include <boost/bind.hpp>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/widgets/styled_background.h>

// Local:
#include "operator_widget.h"
#include "params.h"
#include "part.h"
#include "part_manager.h"


namespace Yuki {

using namespace Haruhi::ScreenLiterals;


OperatorWidget::OperatorWidget (QWidget* parent, unsigned int operator_no, Params::Operator* params, Part* part):
	QWidget (parent),
	_part (part),
	_params (params),
	_stop_widgets_to_params (false),
	_stop_params_to_widgets (false)
{
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	_frequency_numerator = new QSpinBox (this);
	_frequency_numerator->setMinimum (params->frequency_numerator.minimum());
	_frequency_numerator->setMaximum (params->frequency_numerator.maximum());
	_frequency_numerator->setValue (params->frequency_numerator.default_value());
	QObject::connect (_frequency_numerator, SIGNAL (valueChanged (int)), this, SLOT (widgets_to_params()));

	_frequency_denominator = new QSpinBox (this);
	_frequency_denominator->setMinimum (params->frequency_denominator.minimum());
	_frequency_denominator->setMaximum (params->frequency_denominator.maximum());
	_frequency_denominator->setValue (params->frequency_denominator.default_value());
	QObject::connect (_frequency_denominator, SIGNAL (valueChanged (int)), this, SLOT (widgets_to_params()));

	Part::PartControllerProxies* proxies = part->proxies();
	_knob_detune = new Haruhi::Knob (this, proxies->operator_detune[operator_no], "Detune");
	_knob_detune->set_unit_bay (_part->part_manager()->plugin()->unit_bay());

	_octave = new QSpinBox (this);
	_octave->setMinimum (params->octave.minimum());
	_octave->setMaximum (params->octave.maximum());
	_octave->setValue (params->octave.default_value());
	QObject::connect (_octave, SIGNAL (valueChanged (int)), this, SLOT (widgets_to_params()));

	// Force normal text color. For some reason Qt uses white color on light-gray background.
	QLabel* label = new QLabel (QString ("Operator %1").arg (operator_no + 1), this);
	label->setForegroundRole (QPalette::Text);

	QGroupBox* group = new QGroupBox (this);
	QGridLayout* group_layout = new QGridLayout (group);
	group_layout->setMargin (Config::margin());
	group_layout->setSpacing (Config::spacing());
	group_layout->addWidget (new Haruhi::StyledBackground (label, this, 0.3_screen_mm), 0, 0, 1, 4);
	QLabel* f = new QLabel ("Frequency: ", this);
	f->setToolTip ("Relative frequency");
	group_layout->addWidget (f, 1, 0);
	group_layout->addWidget (_frequency_numerator, 1, 1);
	group_layout->addWidget (new QLabel ("÷", this), 1, 2);
	group_layout->addWidget (_frequency_denominator, 1, 3);
	group_layout->addWidget (new QLabel ("Octave:", this), 2, 0, 1, 2);
	group_layout->addWidget (_octave, 2, 1, 1, 3, Qt::AlignRight);
	group_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 3, 0);

	QGridLayout* layout = new QGridLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addWidget (group, 0, 0);
	layout->addWidget (_knob_detune, 0, 1);

	// Update widgets when params change:
	_params->frequency_numerator.on_change.connect (this, &OperatorWidget::post_params_to_widgets);
	_params->frequency_denominator.on_change.connect (this, &OperatorWidget::post_params_to_widgets);
	_params->octave.on_change.connect (this, &OperatorWidget::post_params_to_widgets);
}


OperatorWidget::~OperatorWidget()
{
	delete _knob_detune;
}


void
OperatorWidget::widgets_to_params()
{
	if (_stop_widgets_to_params)
		return;
	_stop_params_to_widgets = true;

	_params->frequency_numerator = _frequency_numerator->value();
	_params->frequency_denominator = _frequency_denominator->value();
	_params->octave = _octave->value();

	_stop_params_to_widgets = false;
}


void
OperatorWidget::params_to_widgets()
{
	if (_stop_params_to_widgets)
		return;
	_stop_widgets_to_params = true;

	_frequency_numerator->setValue (_params->frequency_numerator);
	_frequency_denominator->setValue (_params->frequency_denominator);
	_octave->setValue (_params->octave);

	_stop_widgets_to_params = false;
}


void
OperatorWidget::post_params_to_widgets()
{
	Haruhi::Services::call_out (boost::bind (&OperatorWidget::params_to_widgets, this));
}

} // namespace Yuki

