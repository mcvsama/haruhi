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
	_effect_panel = new QGroupBox (this);

	QGroupBox* bottom_panel = new QGroupBox (this);

	QPushButton* enabled_button = new QPushButton ("&Enabled", bottom_panel);
	enabled_button->setCheckable (true);
	enabled_button->setChecked (true);

	QHBoxLayout* bottom_panel_layout = new QHBoxLayout (bottom_panel);
	bottom_panel_layout->setMargin (Config::Margin);
	bottom_panel_layout->setSpacing (Config::Spacing);
	bottom_panel_layout->addWidget (enabled_button);
	bottom_panel_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::Margin);
	layout->setSpacing (Config::Spacing);
	layout->addWidget (_effect_panel);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
	layout->addWidget (bottom_panel);
}


QWidget*
Effect::parent_widget() const
{
	return _effect_panel;
}

} // namespace MikuruPrivate

