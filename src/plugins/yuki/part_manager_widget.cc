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
#include <functional>

// Qt:
#include <QStackedWidget>
#include <QLayout>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/widgets/knob.h>
#include <haruhi/utility/literals.h>

// Local:
#include "part_manager_widget.h"
#include "part_manager.h"


namespace Yuki {

PartManagerWidget::Placeholder::Placeholder (QWidget* parent):
	QWidget (parent)
{
	auto layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	layout->addWidget (new QLabel ("Add parts with buttons above", this));
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
}


PartManagerWidget::PartManagerWidget (QWidget* parent, PartManager* part_manager):
	QWidget (parent),
	_part_manager (part_manager),
	_stop_widgets_to_params (false),
	_stop_params_to_widgets (false)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	Params::Main* main_params = _part_manager->main_params();

	// Knobs:

	PartManager::MainProxies* proxies = _part_manager->proxies();

	_knob_volume		= std::make_unique<Haruhi::Knob> (this, &proxies->volume, "Level dB");
	_knob_panorama		= std::make_unique<Haruhi::Knob> (this, &proxies->panorama, "Panorama");
	_knob_detune		= std::make_unique<Haruhi::Knob> (this, &proxies->detune, "Detune");
	_knob_stereo_width	= std::make_unique<Haruhi::Knob> (this, &proxies->stereo_width, "Stereo");

	_knob_volume->set_volume_scale (true, M_E);
	auto level_color = QColor (0x00, 0xbb, 0x00);
	_knob_volume->set_color_hint (level_color);
	_knob_panorama->set_color_hint (level_color);
	_knob_stereo_width->set_color_hint (level_color);
	auto pitch_color = QColor (0x00, 0x6e, 0xff);
	_knob_detune->set_color_hint (pitch_color);

	// Polyphony:

	_polyphony = std::make_unique<QSpinBox> (this);
	_polyphony->setMinimum (main_params->polyphony.minimum());
	_polyphony->setMaximum (main_params->polyphony.maximum());
	_polyphony->setValue (main_params->polyphony.get());
	QObject::connect (_polyphony.get(), SIGNAL (valueChanged (int)), this, SLOT (widgets_to_params()));

	// Oversampling:

	_oversampling = std::make_unique<QSpinBox> (this);
	_oversampling->setMinimum (main_params->oversampling.minimum());
	_oversampling->setMaximum (main_params->oversampling.maximum());
	_oversampling->setValue (_part_manager->main_params()->oversampling.get());
	_oversampling->setSpecialValueText ("None");
	_oversampling->setSuffix ("x");
	QObject::connect (_oversampling.get(), SIGNAL (valueChanged (int)), this, SLOT (widgets_to_params()));

	// Top buttons:

	auto buttons_widget = new QWidget (this);

	_add_part_button = std::make_unique<QPushButton> (Resources::Icons16::add(), "Add part", this);
	_add_part_button->setIconSize (Resources::Icons16::haruhi().size());
	_add_part_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);
	_add_part_button->setFlat (true);
	QObject::connect (_add_part_button.get(), SIGNAL (clicked()), this, SLOT (add_part()));

	_remove_part_button = std::make_unique<QPushButton> (Resources::Icons16::remove(), "", this);
	_remove_part_button->setIconSize (Resources::Icons16::haruhi().size());
	_remove_part_button->setToolTip ("Remove current part");
	_remove_part_button->setFlat (true);
	QObject::connect (_remove_part_button.get(), SIGNAL (clicked()), this, SLOT (remove_current_part()));

	// Part tabs:

	_tabs = std::make_unique<TabWidget> (this);
	_tabs->setMovable (true);
	_tabs->setCornerWidget (buttons_widget, Qt::TopRightCorner);
	_tabs->setIconSize (Resources::Icons16::haruhi().size() * 1.25);
	QObject::connect (_tabs->tabBar(), SIGNAL (tabMoved (int, int)), this, SLOT (tab_moved (int, int)));

	_placeholder = std::make_unique<Placeholder> (this);

	// Layouts:

	auto buttons_layout = new QGridLayout (buttons_widget);
	buttons_layout->setMargin (0);
	buttons_layout->setSpacing (Config::spacing());
	buttons_layout->addWidget (_add_part_button.get(), 0, 0);
	buttons_layout->addWidget (_remove_part_button.get(), 0, 1);
	buttons_layout->addItem (new QSpacerItem (0, Config::spacing(), QSizePolicy::Fixed, QSizePolicy::Fixed), 1, 0, 1, 2);

	auto main_layout = new QVBoxLayout();
	main_layout->setMargin (0);
	main_layout->setSpacing (Config::spacing());
	main_layout->addWidget (_knob_volume.get());
	main_layout->addWidget (_knob_panorama.get());
	main_layout->addWidget (_knob_stereo_width.get());
	main_layout->addWidget (_knob_detune.get());
	main_layout->addItem (new QSpacerItem (0, Config::spacing(), QSizePolicy::Fixed, QSizePolicy::Fixed));
	main_layout->addWidget (new QLabel ("Max voices:", this));
	main_layout->addWidget (_polyphony.get());
	main_layout->addWidget (new QLabel ("Ovrsmpling:", this));
	main_layout->addWidget (_oversampling.get());
	main_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	auto layout = new QHBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addLayout (main_layout);
	layout->addWidget (_tabs.get());

	update_widgets();

	_part_manager->part_added.connect (this, &PartManagerWidget::add_part);
	_part_manager->part_removed.connect (this, &PartManagerWidget::remove_part);
	_part_manager->part_updated.connect (this, &PartManagerWidget::update_part);

	_part_manager->main_params()->polyphony.on_change.connect (this, &PartManagerWidget::post_params_to_widgets);
}


Plugin*
PartManagerWidget::plugin() const
{
	return _part_manager->plugin();
}


void
PartManagerWidget::unit_bay_assigned()
{
	for (auto* k: { _knob_volume.get(), _knob_panorama.get(), _knob_detune.get(), _knob_stereo_width.get() })
		k->set_unit_bay (plugin()->unit_bay());
}


void
PartManagerWidget::add_part (Part* part)
{
	PartWidget* pw = new PartWidget (this, part);
	part->set_widget (pw);
	_tabs->addTab (pw, Resources::Icons16::parts(), "Part %1"_qstr.arg (part->id()));
	_tabs->setCurrentWidget (pw);
	update_widgets();
}


void
PartManagerWidget::remove_part (Part* part)
{
	for (int i = 0, n = _tabs->count(); i < n; ++i)
	{
		auto pw = dynamic_cast<PartWidget*> (_tabs->widget (i));
		assert (pw);
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
PartManagerWidget::update_part (Part* part)
{
	for (int i = 0, n = _tabs->count(); i < n; ++i)
	{
		auto pw = dynamic_cast<PartWidget*> (_tabs->widget (i));
		assert (pw);
		if (pw->part() == part)
			_tabs->setTabText (i, QString ("Part %1").arg (part->id()));
	}
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
		auto pw = dynamic_cast<PartWidget*> (_tabs->currentWidget());
		assert (pw);
		_part_manager->remove_part (pw->part());
	}
}


void
PartManagerWidget::tab_moved (int old_position, int new_position)
{
	_part_manager->set_part_position (old_position, new_position);
}


void
PartManagerWidget::widgets_to_params()
{
	if (_stop_widgets_to_params)
		return;
	_stop_params_to_widgets = true;

	_part_manager->main_params()->polyphony = _polyphony->value();
	_part_manager->main_params()->oversampling = _oversampling->value();

	_stop_params_to_widgets = false;
}


void
PartManagerWidget::params_to_widgets()
{
	if (_stop_params_to_widgets)
		return;
	_stop_widgets_to_params = true;

	_polyphony->setValue (_part_manager->main_params()->polyphony);
	_oversampling->setValue (_part_manager->main_params()->oversampling);

	_stop_widgets_to_params = false;

	update_widgets();
}


void
PartManagerWidget::post_params_to_widgets()
{
	Haruhi::Services::call_out (std::bind (&PartManagerWidget::params_to_widgets, this));
}


void
PartManagerWidget::update_widgets()
{
	unsigned int parts_number = _tabs->count() > 0;

	_remove_part_button->setEnabled (parts_number > 0);

	if (_part_manager->parts_number() == 0)
		_tabs->addTab (_placeholder.get(), Resources::Icons16::add(), "→");
	else
	{
		for (int i = 0, n = _tabs->count(); i < n; ++i)
		{
			if (dynamic_cast<Placeholder*> (_tabs->widget (i)))
				_tabs->removeTab (i--);
		}
	}
}

} // namespace Yuki

