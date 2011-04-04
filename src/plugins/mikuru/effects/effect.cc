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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>

// Local:
#include "../mikuru.h"
#include "effect.h"


namespace MikuruPrivate {

Effect::Effect (int id, QString const& id_group, Mikuru* mikuru, QString const& port_group_name, Params::Effect* params, QWidget* parent):
	QWidget (parent),
	_mikuru (mikuru),
	_id_group (id_group),
	_params (params)
{
	_id = (id == 0) ? _mikuru->allocate_id (_id_group.toStdString()) : _mikuru->reserve_id (_id_group.toStdString(), id);

	_effect_panel = new QWidget (this);
	_effect_panel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Maximum);

	QGroupBox* common_panel = new QGroupBox (this);

	_enabled_button = new QPushButton ("&Enabled", common_panel);
	_enabled_button->setCheckable (true);
	_enabled_button->setChecked (true);
	QObject::connect (_enabled_button, SIGNAL (toggled (bool)), this, SLOT (update_params()));

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	_port_group = new Haruhi::PortGroup (_mikuru->graph(), QString ("%1 %2").arg (port_group_name).arg (this->id()).toStdString());
	_port_wet = new Haruhi::EventPort (_mikuru, "Wet", Haruhi::Port::Input, _port_group);
	_port_panorama = new Haruhi::EventPort (_mikuru, "Panorama", Haruhi::Port::Input, _port_group);
	if (_mikuru->graph())
		_mikuru->graph()->unlock();

	_knob_wet = new Haruhi::Knob (this, _port_wet, &_params->wet, "Wet", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Effect::Wet, 100), 2);
	_knob_panorama = new Haruhi::Knob (this, _port_panorama, &_params->panorama, "Panorama", HARUHI_MIKURU_PARAMS_FOR_KNOB_WITH_STEPS (Params::Effect::Panorama, 100), 2);

	_knob_wet->set_unit_bay (_mikuru->unit_bay());
	_knob_panorama->set_unit_bay (_mikuru->unit_bay());

	QGridLayout* common_panel_layout = new QGridLayout (common_panel);
	common_panel_layout->setMargin (Config::Margin);
	common_panel_layout->setSpacing (Config::Spacing);
	common_panel_layout->addWidget (_enabled_button, 0, 0);
	common_panel_layout->addWidget (new QLabel ("(Here will be effect presets support)"), 0, 1, 1, 1, Qt::AlignHCenter);
	common_panel_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 0, 1);

	QGridLayout* layout = new QGridLayout (this);
	layout->setMargin (Config::Margin);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (common_panel, 0, 0, 1, 3);
	layout->addWidget (_effect_panel, 1, 0, 1, 1, Qt::AlignTop);
	layout->addWidget (_knob_wet, 1, 1, 1, 1, Qt::AlignTop | Qt::AlignRight);
	layout->addWidget (_knob_panorama, 1, 2, 1, 1, Qt::AlignTop | Qt::AlignRight);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding), 2, 0);
	layout->setRowStretch (2, 1);
}


Effect::~Effect()
{
	_mikuru->free_id (_id_group.toStdString(), _id);

	delete _knob_wet;

	if (_mikuru->graph())
		_mikuru->graph()->lock();
	delete _port_wet;
	delete _port_panorama;
	delete _port_group;
	if (_mikuru->graph())
		_mikuru->graph()->unlock();

	delete _params;
}


void
Effect::process_events()
{
	_knob_wet->process_events();
	_knob_panorama->process_events();
}


void
Effect::load_params()
{
	// Copy params:
	Params::Effect p (*_params);

	_enabled_button->setChecked (p.enabled);
}


void
Effect::update_params()
{
	_params->enabled.set (_enabled_button->isChecked());
}

} // namespace MikuruPrivate

