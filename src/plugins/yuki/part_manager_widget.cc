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

PartManagerWidget::Placeholder::Placeholder (QWidget* parent):
	QWidget (parent)
{
	QHBoxLayout* layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	layout->addWidget (new QLabel ("Add parts with buttons above", this));
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
}


PartManagerWidget::PartManagerWidget (QWidget* parent, PartManager* part_manager):
	QWidget (parent),
	_part_manager (part_manager)
{
	// Knobs:

	PartManager::MainProxies* proxies = _part_manager->proxies();

	_knob_volume		= new Haruhi::Knob (this, &proxies->volume, "Volume dB");
	_knob_panorama		= new Haruhi::Knob (this, &proxies->panorama, "Panorama");
	_knob_detune		= new Haruhi::Knob (this, &proxies->detune, "Detune");
	_knob_stereo_width	= new Haruhi::Knob (this, &proxies->stereo_width, "Stereo");

	_knob_volume->set_volume_scale (true, M_E);

	// Top buttons:

	QWidget* buttons_widget = new QWidget (this);

	_add_part_button = new QPushButton (Resources::Icons16::add(), "Add part", this);
	_add_part_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);
	QObject::connect (_add_part_button, SIGNAL (clicked()), this, SLOT (add_part()));

	_remove_part_button = new QPushButton (Resources::Icons16::remove(), "", this);
	QObject::connect (_remove_part_button, SIGNAL (clicked()), this, SLOT (remove_current_part()));
	QToolTip::add (_remove_part_button, "Remove current part");

	// Part tabs:

	_tabs = new QTabWidget (this);
	_tabs->setMovable (true);
	_tabs->setCornerWidget (buttons_widget, Qt::TopRightCorner);
	_tabs->setIconSize (QSize (32, 22));

	_placeholder = new Placeholder (this);

	// Layouts:

	QGridLayout* buttons_layout = new QGridLayout (buttons_widget);
	buttons_layout->setMargin (0);
	buttons_layout->setSpacing (Config::Spacing);
	buttons_layout->addWidget (_add_part_button, 0, 0);
	buttons_layout->addWidget (_remove_part_button, 0, 1);
	buttons_layout->addItem (new QSpacerItem (0, Config::Spacing, QSizePolicy::Fixed, QSizePolicy::Fixed), 1, 0, 1, 2);

	QVBoxLayout* main_layout = new QVBoxLayout();
	main_layout->setMargin (0);
	main_layout->setSpacing (Config::Spacing);
	main_layout->addWidget (_knob_volume);
	main_layout->addWidget (_knob_panorama);
	main_layout->addWidget (_knob_detune);
	main_layout->addWidget (_knob_stereo_width);
	main_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	QHBoxLayout* layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::Spacing);
	layout->addLayout (main_layout);
	layout->addWidget (_tabs);

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
	unsigned int parts_number = _tabs->count() > 0;

	_remove_part_button->setEnabled (parts_number > 0);

	if (_part_manager->parts_number() == 0)
		_tabs->addTab (_placeholder, Resources::Icons16::add(), "→");
	else
	{
		for (int i = 0, n = _tabs->count(); i < n; ++i)
		{
			Placeholder* ph = dynamic_cast<Placeholder*> (_tabs->widget (i));
			if (ph)
				_tabs->removeTab (i--);
		}
	}
}

} // namespace Yuki

