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
#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>

// Local:
#include "effect.h"


namespace MikuruPrivate {

Effect::Effect (QWidget* parent):
	QWidget (parent)
{
	_effect_panel = new QWidget (this);
	_effect_panel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Maximum);

	QGroupBox* common_panel = new QGroupBox (this);

	_enabled_button = new QPushButton ("&Enabled", common_panel);
	_enabled_button->setCheckable (true);
	_enabled_button->setChecked (true);

	QHBoxLayout* common_panel_layout = new QHBoxLayout (common_panel);
	common_panel_layout->setMargin (Config::Margin);
	common_panel_layout->setSpacing (Config::Spacing);
	common_panel_layout->addWidget (_enabled_button);
	common_panel_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::Margin);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (common_panel);
	layout->addWidget (_effect_panel);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
}


QWidget*
Effect::parent_widget() const
{
	return _effect_panel;
}


void
Effect::load_params()
{
	// Copy params:
	Params::Effect p (effect_params());

	_enabled_button->setChecked (p.enabled);
}


void
Effect::update_params()
{
	effect_params().enabled.set (_enabled_button->isChecked());
}

} // namespace MikuruPrivate

