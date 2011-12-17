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
#include <QtGui/QStackedWidget>
#include <QtGui/QToolTip>
#include <QtGui/QLayout>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/widgets/knob.h>

// Local:
#include "part_manager_widget.h"
#include "part_manager.h"


namespace Yuki {

PartManagerWidget::PartManagerWidget (QWidget* parent, PartManager* part_manager):
	QWidget (parent),
	_part_manager (part_manager),
	_prevent_recursion (false)
{
	_main = new QWidget (this);

	// Knobs:

	PartManager::MainProxies* proxies = _part_manager->proxies();

	_knob_volume		= new Haruhi::Knob (this, &proxies->volume, "Volume dB");
	_knob_panorama		= new Haruhi::Knob (this, &proxies->panorama, "Panorama");
	_knob_detune		= new Haruhi::Knob (this, &proxies->detune, "Detune");
	_knob_stereo_width	= new Haruhi::Knob (this, &proxies->stereo_width, "Stereo");

	_knob_volume->set_volume_scale (true, M_E);

	QHBoxLayout* main_layout = new QHBoxLayout (_main);
	main_layout->setMargin (0);
	main_layout->setSpacing (Config::Spacing);
	main_layout->addWidget (_knob_volume);
	main_layout->addWidget (_knob_panorama);
	main_layout->addWidget (_knob_detune);
	main_layout->addWidget (_knob_stereo_width);

	// Top buttons:

	_show_main_button = new QPushButton (Resources::Icons16::main(), "Main controls", this);
	_show_main_button->setCheckable (true);
	QObject::connect (_show_main_button, SIGNAL (toggled (bool)), this, SLOT (show_main()));

	_show_tabs_button = new QPushButton (Resources::Icons16::parts(), "Parts", this);
	_show_tabs_button->setCheckable (true);
	QObject::connect (_show_tabs_button, SIGNAL (toggled (bool)), this, SLOT (show_parts()));

	_add_part_button = new QPushButton (Resources::Icons16::add(), "Add part", this);
	QObject::connect (_add_part_button, SIGNAL (clicked()), this, SLOT (add_part()));

	_remove_part_button = new QPushButton (Resources::Icons16::remove(), "", this);
	QObject::connect (_remove_part_button, SIGNAL (clicked()), this, SLOT (remove_current_part()));
	QToolTip::add (_remove_part_button, "Remove current part");

	// Part tabs:

	_tabs = new QTabWidget (this);
	_tabs->setMovable (true);

	_stack = new QStackedWidget (this);
	_stack->addWidget (_main);
	_stack->addWidget (_tabs);

	// Layouts:

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->setMargin (0);
	buttons_layout->setSpacing (Config::Spacing);
	buttons_layout->addWidget (_show_main_button);
	buttons_layout->addWidget (_show_tabs_button);
	buttons_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (_add_part_button);
	buttons_layout->addWidget (_remove_part_button);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addLayout (buttons_layout);
	layout->addWidget (_stack);

	show_main();
	update_widgets();

	_part_manager->part_added.connect (this, &PartManagerWidget::add_part);
	_part_manager->part_removed.connect (this, &PartManagerWidget::remove_part);
}


Plugin*
PartManagerWidget::plugin() const
{
	return _part_manager->plugin();
}


void
PartManagerWidget::unit_bay_assigned()
{
	Haruhi::Knob* all_knobs[] = { _knob_volume, _knob_panorama, _knob_detune, _knob_stereo_width };
	for (Haruhi::Knob** k = all_knobs; k != all_knobs + ARRAY_SIZE (all_knobs); ++k)
		(*k)->set_unit_bay (plugin()->unit_bay());
}


void
PartManagerWidget::add_part (Part* part)
{
	PartWidget* pw = new PartWidget (this, part);
	part->set_widget (pw);
	_tabs->addTab (pw, Resources::Icons16::wave_sine(), QString ("Part %1").arg (part->id()));
	_tabs->setCurrentWidget (pw);
	update_widgets();
	show_parts();
}


void
PartManagerWidget::remove_part (Part* part)
{
	for (int i = 0, n = _tabs->count(); i < n; ++i)
	{
		PartWidget* pw = dynamic_cast<PartWidget*> (_tabs->widget (i));
		assert (pw != 0);
		if (pw->part() == part)
		{
			_tabs->removeTab (i);
			delete pw;
			break;
		}
	}
	update_widgets();
}


void
PartManagerWidget::show_main()
{
	if (_prevent_recursion)
		return;
	_prevent_recursion = true;
	_stack->setCurrentWidget (_main);
	_show_main_button->setChecked (true);
	_show_tabs_button->setChecked (false);
	_prevent_recursion = false;
	update_widgets();
}


void
PartManagerWidget::show_parts()
{
	if (_prevent_recursion)
		return;
	_prevent_recursion = true;
	_stack->setCurrentWidget (_tabs);
	_show_main_button->setChecked (false);
	_show_tabs_button->setChecked (true);
	_prevent_recursion = false;
	update_widgets();
}


void
PartManagerWidget::add_part()
{
	_part_manager->add_part();
}


void
PartManagerWidget::remove_current_part()
{
	if (_tabs->count() > 0)
	{
		PartWidget* pw = dynamic_cast<PartWidget*> (_tabs->currentPage());
		assert (pw != 0);
		if (pw)
			_part_manager->remove_part (pw->part());
	}
}


void
PartManagerWidget::update_widgets()
{
	_remove_part_button->setEnabled (_show_tabs_button->isChecked() && _tabs->count() > 0);
}

} // namespace Yuki

