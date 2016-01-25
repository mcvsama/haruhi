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
#include <QtGui/QLayout>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "part_modulator_widget.h"
#include "part_widget.h"
#include "part.h"
#include "part_manager.h"


namespace Yuki {

using namespace Haruhi::ScreenLiterals;


PartModulatorWidget::PartModulatorWidget (QWidget* parent, PartWidget* part_widget, Part* part):
	QWidget (parent),
	_part_widget (part_widget),
	_part (part)
{
	// Shorthand links:
	Part::PartControllerProxies* proxies = _part->proxies();
	Params::Part* pp = _part->part_params();

	// "+ 1" is for output to main oscillator:
	_fm_matrix_knobs.resize (Params::Part::OperatorsNumber + 1);
	_am_matrix_knobs.resize (Params::Part::OperatorsNumber + 1);
	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
	{
		_fm_matrix_knobs[o].resize (Params::Part::OperatorsNumber);
		_am_matrix_knobs[o].resize (Params::Part::OperatorsNumber);
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
		{
			QString t = QString ("%1").arg (o + 1);
			if (o == 3)
				t = "M";
			_fm_matrix_knobs[o][i] = std::make_unique<Haruhi::Knob> (this, proxies->fm_matrix[o][i].get(), QString ("%1 → %2").arg (i + 1).arg (t));
			_am_matrix_knobs[o][i] = std::make_unique<Haruhi::Knob> (this, proxies->am_matrix[o][i].get(), QString ("%1 → %2").arg (i + 1).arg (t));
			_fm_matrix_knobs[o][i]->set_narrow (true);
			_am_matrix_knobs[o][i]->set_narrow (true);
		}
	}

	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
	{
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
		{
			_fm_matrix_knobs[o][i]->set_unit_bay (_part->part_manager()->plugin()->unit_bay());
			_am_matrix_knobs[o][i]->set_unit_bay (_part->part_manager()->plugin()->unit_bay());
		}
	}

	// Operator widgets:
	_operator_1 = new OperatorWidget (this, 0, &_part->part_params()->operators[0], _part);
	_operator_2 = new OperatorWidget (this, 1, &_part->part_params()->operators[1], _part);
	_operator_3 = new OperatorWidget (this, 2, &_part->part_params()->operators[2], _part);

	// Modulator enabled:
	_modulator_enabled = new QCheckBox ("Modulator enabled", this);
	_modulator_enabled->setChecked (pp->modulator_enabled);
	QObject::connect (_modulator_enabled, SIGNAL (toggled (bool)), this, SLOT (widgets_to_oscillator_params()));
	QObject::connect (_modulator_enabled, SIGNAL (toggled (bool)), this, SLOT (update_widgets()));

	// Layouts:

	QLabel* fm_modulation_matrix_label = create_modulator_label ("FM modulation matrix:");
	QLabel* am_modulation_matrix_label = create_modulator_label ("AM modulation matrix:");
	fm_modulation_matrix_label->setFixedHeight (2.2f * Haruhi::Services::y_pixels_per_point() * fm_modulation_matrix_label->font().pointSize());
	am_modulation_matrix_label->setFixedHeight (2.2f * Haruhi::Services::y_pixels_per_point() * fm_modulation_matrix_label->font().pointSize());

	QGridLayout* modulator_layout = new QGridLayout (this);
	modulator_layout->setMargin (Config::margin());
	modulator_layout->setSpacing (Config::spacing());
	modulator_layout->addWidget (_modulator_enabled, 0, 0);
	modulator_layout->addWidget (_operator_1, 1, 0);
	modulator_layout->addWidget (_operator_2, 2, 0);
	modulator_layout->addWidget (_operator_3, 3, 0);
	modulator_layout->addWidget (create_modulator_label ("→"), 1, 1, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("→"), 2, 1, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("→"), 3, 1, 1, 1, Qt::AlignCenter);
	modulator_layout->addItem (new QSpacerItem (Config::spacing() * 4, 0, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 1);
	modulator_layout->addWidget (fm_modulation_matrix_label, 0, 2, 1, 4, Qt::AlignLeft);
	modulator_layout->addItem (new QSpacerItem (Config::spacing() * 4, 0, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 6);
	modulator_layout->addWidget (am_modulation_matrix_label, 0, 7, 1, 4, Qt::AlignLeft);
	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
			modulator_layout->addWidget (_fm_matrix_knobs[o][i].get(), i + 1, o + 2);
	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
			modulator_layout->addWidget (_am_matrix_knobs[o][i].get(), i + 1, o + 7);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 1"), 4, 2, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 2"), 4, 3, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 3"), 4, 4, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op M"), 4, 5, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 1"), 4, 7, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 2"), 4, 8, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op 3"), 4, 9, 1, 1, Qt::AlignCenter);
	modulator_layout->addWidget (create_modulator_label ("↳ Op M"), 4, 10, 1, 1, Qt::AlignCenter);

	modulator_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 5, 0);
	modulator_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 11);
}


void
PartModulatorWidget::widgets_to_oscillator_params()
{
	Params::Part* pp = _part->part_params();

	pp->modulator_enabled = _modulator_enabled->isChecked();
}


void
PartModulatorWidget::update_widgets()
{
	const bool mod_enabled = _modulator_enabled->isChecked();
	_operator_1->setEnabled (mod_enabled);
	_operator_2->setEnabled (mod_enabled);
	_operator_3->setEnabled (mod_enabled);

	for (unsigned int o = 0; o < Params::Part::OperatorsNumber + 1; ++o)
	{
		for (unsigned int i = 0; i < Params::Part::OperatorsNumber; ++i)
		{
			_fm_matrix_knobs[o][i]->setEnabled (mod_enabled);
			_am_matrix_knobs[o][i]->setEnabled (mod_enabled);
		}
	}

	for (auto* ml: _modulator_labels)
		ml->setEnabled (mod_enabled);
}


void
PartModulatorWidget::params_to_widgets()
{
	Params::Part* pp = _part->part_params();

	_modulator_enabled->setChecked (pp->modulator_enabled);
}


QLabel*
PartModulatorWidget::create_modulator_label (QString const& text)
{
	QLabel* label = new QLabel (text, this);
	_modulator_labels.push_back (label);
	return label;
}

} // namespace Yuki

