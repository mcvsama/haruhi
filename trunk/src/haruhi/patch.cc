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

// Standard:
#include <cstddef>
#include <algorithm>
#include <map>

// Qt:
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include <Qt3Support/Q3Header>
#include <Qt3Support/Q3PopupMenu>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/session.h>
#include <haruhi/presetable.h>
#include <haruhi/connections_dump.h>
#include <haruhi/components/presets_manager/presets_manager.h>

// Local:
#include "patch.h"


namespace Haruhi {

namespace PatchPrivate {

ConnectionsTab::ConnectionsTab (Patch* patch, QWidget* parent):
	QWidget (parent),
	_patch (patch)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setContentsMargins (Config::margin, Config::margin, Config::margin, Config::margin);

	QVBoxLayout* layout = new QVBoxLayout (this, 0, Config::spacing);
	QHBoxLayout* top_layout = new QHBoxLayout (layout, Config::spacing);

	top_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	_ports_connector = new PortsConnector (_patch, this);
	_ports_connector->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	layout->addWidget (_ports_connector);
}


ConnectionsTab::~ConnectionsTab()
{
	_ports_connector->reparent (0, QPoint (0, 0), false);
	delete _ports_connector;
}


UnitTab::UnitTab (Patch* patch, QWidget* parent, Unit* unit):
	QWidget (parent),
	_patch (patch),
	_unit (unit)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	bool unit_is_presetable = dynamic_cast<Presetable*> (unit);

	QWidget* bar = new QWidget (this);
	_stack = new QStackedWidget (this);

	_menu = new Q3PopupMenu (this);
	_menu->insertItem (Config::Icons16::remove(), "Unload", this, SLOT (unload()));

	// Title/menu button:
	QPushButton* title_button = new QPushButton (QString::fromStdString (_unit->title()), bar);
	title_button->clearFocus();
	title_button->setFlat (true);
	title_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	title_button->setPaletteForegroundColor (QColor (0xff, 0xff, 0xff));
	title_button->setPaletteBackgroundColor (QColor (0x00, 0x2A, 0x5B));
	title_button->setPopup (_menu);
	QFont font (QApplication::font());
	font.setWeight (QFont::Black);
	title_button->setFont (font);

	// Presetable?
	if (unit_is_presetable)
	{
		_presets_manager = new PresetsManager (_unit, this);
		QObject::connect (_presets_manager, SIGNAL (preset_selected (const QString&, const QString&)),
						  this, SLOT (set_preset (const QString&, const QString&)));

		// Preset label:
		_preset_name = new QLabel ("", bar);
		QFont font (QApplication::font());
		font.setWeight (QFont::Black);
		_preset_name->setFont (font);

		// Presets button:
		_presets_button = new QPushButton ("Presets", bar);
		_presets_button->clearFocus();
		_presets_button->setFlat (true);
		_presets_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		_presets_button->setPaletteForegroundColor (QColor (0xff, 0xff, 0xff));
		_presets_button->setPaletteBackgroundColor (QColor (0x00, 0x2A, 0x5B));
		QObject::connect (_presets_button, SIGNAL (clicked()), this, SLOT (presets()));

		// Presets manager/favoriting:

		_favorite_checkbox = new QCheckBox ("Favorite", bar);
		_favorite_checkbox->setEnabled (false);
		QObject::connect (_favorite_checkbox, SIGNAL (toggled (bool)), this, SLOT (favorited (bool)));
	}

	// Layouts:
	QVBoxLayout* layout = new QVBoxLayout (this, Config::margin, Config::spacing);
	QHBoxLayout* bar_layout = new QHBoxLayout (bar, 0, Config::spacing);
	bar->setPaletteForegroundColor (QColor (0xff, 0xff, 0xff));
	bar->setPaletteBackgroundColor (QColor (0x00, 0x2A, 0x5B));
	bar->setAutoFillBackground (true);

	bar_layout->addWidget (title_button);
	if (unit_is_presetable)
	{
		bar_layout->addItem (new QSpacerItem (5, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
		bar_layout->addWidget (_preset_name);
		bar_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
		bar_layout->addWidget (_favorite_checkbox);
		bar_layout->addItem (new QSpacerItem (10, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
		bar_layout->addWidget (_presets_button);
	}
	else
		bar_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	_unit->reparent (_stack, QPoint(), true);
	if (unit_is_presetable)
		_stack->addWidget (_presets_manager);
	_stack->addWidget (_unit);
	_stack->setCurrentWidget (_unit);

	layout->addWidget (bar);
	layout->addWidget (_stack);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
}


UnitTab::~UnitTab()
{
	delete _menu;
}


void
UnitTab::unparent_unit()
{
	_unit->reparent (0, QPoint (0, 0), false);
}


void
UnitTab::unload()
{
	_patch->unload_unit (_unit);
}


void
UnitTab::presets()
{
	if (_stack->currentWidget() == _presets_manager)
	{
		_stack->setCurrentWidget (_unit);
		_presets_button->setPaletteForegroundColor (QColor (0xff, 0xff, 0xff));
		_presets_button->setPaletteBackgroundColor (QColor (0x00, 0x2A, 0x5B));
		_presets_button->setAutoFillBackground (true);
	}
	else
	{
		_stack->setCurrentWidget (_presets_manager);
		_presets_button->setPaletteForegroundColor (QColor (0xff, 0xff, 0xff));
		_presets_button->setPaletteBackgroundColor (QColor (0x0c, 0x62, 0xf8));
		_presets_button->setAutoFillBackground (true);
	}
}


void
UnitTab::set_preset (QString const& uuid, QString const& name)
{
	if (_presets_manager)
	{
		_preset_uuid = uuid;
		_preset_name->setText (name);
		_favorite_checkbox->setEnabled (!uuid.isNull());
		_favorite_checkbox->setChecked (_presets_manager->is_favorite (uuid));
	}
}


void
UnitTab::favorited (bool set)
{
	if (_presets_manager)
		_presets_manager->set_favorite (_preset_uuid, _preset_name->text(), set);
}

} // namespace PatchPrivate


namespace Private = PatchPrivate;


Patch::Patch (Session* session, std::string const& title, QWidget* parent):
	UnitBay (session, "", title, 0, parent)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_units_menu = create_units_menu();

	QWidget* add_unit_frame = new QWidget (this);

	QPushButton* add_unit_button = new QPushButton (Config::Icons16::add(), "Load unit", add_unit_frame);
	add_unit_button->setFlat (true);
	add_unit_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);
	add_unit_button->setPopup (_units_menu);

	QVBoxLayout* add_unit_layout = new QVBoxLayout (add_unit_frame, 0, 0);
	add_unit_layout->addItem (new QSpacerItem (0, Config::spacing, QSizePolicy::Fixed, QSizePolicy::Expanding));
	add_unit_layout->addWidget (add_unit_button);

	_layout = new QVBoxLayout (this, 0, Config::spacing);

		_tabs = new QTabWidget (this);
		_tabs->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
		_tabs->setTabPosition (QTabWidget::South);
		_tabs->setIconSize (QSize (32, 22));
		_tabs->setMovable (true);
		_tabs->setCornerWidget (add_unit_frame, Qt::BottomRightCorner);

			_connections_tab = new Private::ConnectionsTab (this, _tabs);

		_tabs->addTab (_connections_tab, Config::Icons22::connections(), "Connections");

	_layout->addWidget (_tabs);
}


Patch::~Patch()
{
	// Delete _connections_tab manually, to early delete PortsConnector:
	_connections_tab->ports_connector()->reparent (0, QPoint (0, 0), false);
	delete _connections_tab;
	delete _units_menu;

	while (!units().empty())
	{
		Units::iterator u = units().begin();
		(*u)->hide();
		(*u)->reparent (0, QPoint (0, 0), false);
		// TODO use unloader for deleting objects:
		delete *u; // Deleting unit will automatically remove it from units list (via graph notification).
	}
	units().clear();
}


Unit*
Patch::load_unit (QString const& urn)
{
	Unit* unit = session()->unit_loader()->load (urn.toStdString());
	if (unit)
	{
		// If unit isn't already in units list, insert it:
		if (units().find (unit) == units().end())
		{
			units().insert (unit);
			_connections_tab->ports_connector()->insert_unit (unit);
		}

		// If unit is UnitBayAware, setup self as UnitBay:
		UnitBayAware* unit_bay_aware_unit = dynamic_cast<UnitBayAware*> (unit);
		if (unit_bay_aware_unit)
			unit_bay_aware_unit->set_unit_bay (this);

		// Create unit frame:
		Private::UnitTab* unit_tab = new Private::UnitTab (this, _tabs, unit);
		// TODO patch should give a hint to unit how should it call itself in Core::Graph:
		_tabs->addTab (unit_tab, Config::Icons22::spacer(), QString ("%1: %2").arg (1).arg (QString::fromStdString (unit->title())));
		_units_to_frames_map[unit] = unit_tab;
	}
	return unit;
}


void
Patch::unload_unit (Unit* unit)
{
	UnitsToFramesMap::iterator u = _units_to_frames_map.find (unit);
	if (u != _units_to_frames_map.end())
	{
		Private::UnitTab* unit_tab = u->second;
		unit_tab->unparent_unit();
		// Remove unit from ports connector:
		_connections_tab->ports_connector()->remove_unit (unit);
		// Remove unit from unit bay:
		units().erase (unit);
		// Unload unit:
		session()->unit_loader()->unload (unit);
		// Dispose of tab:
		unit_tab->reparent (0, QPoint (0, 0), false);
		delete unit_tab;
		_units_to_frames_map.erase (u);
	}
}


void
Patch::save_state (QDomElement& element) const
{
	QDomElement units = element.ownerDocument().createElement ("units");

	// Tabs sorted by their tab-position:
	std::multimap<int, QDomElement> sorted_units;
	for (Units::const_iterator u = this->units().begin(); u != this->units().end(); ++u)
	{
		QDomElement unit = element.ownerDocument().createElement ("unit");
		// Unit attributes:
		unit.setAttribute ("urn", QString::fromStdString ((*u)->urn()));
		unit.setAttribute ("title", QString::fromStdString ((*u)->title()));
		unit.setAttribute ("id", QString ("%1").arg ((*u)->id()));
		// Presetable?
		Presetable* presetable = dynamic_cast<Presetable*> (*u);
		if (presetable)
		{
			unit.setAttribute ("preset-name", _units_to_frames_map.find (*u)->second->preset_name());
			unit.setAttribute ("preset-uuid", _units_to_frames_map.find (*u)->second->preset_uuid());
		}
		// SaveableState?
		SaveableState* saveable_state = dynamic_cast<SaveableState*> (*u);
		if (saveable_state)
			saveable_state->save_state (unit);
		// Tab position:
		sorted_units.insert (std::make_pair (unit_tab_position (*u), unit));
	}
	for (std::multimap<int, QDomElement>::iterator u = sorted_units.begin(); u != sorted_units.end(); ++u)
		units.appendChild (u->second);

	QDomElement connections = element.ownerDocument().createElement ("connections");
	ConnectionsDump connections_dump;
	connections_dump.insert_units (this->units().begin(), this->units().end());
	connections_dump.insert_unit (session()->audio_backend());
	connections_dump.insert_unit (session()->event_backend());
	connections_dump.save();
	connections_dump.save_state (connections);

	element.appendChild (units);
	element.appendChild (connections);
}


void
Patch::load_state (QDomElement const& element)
{
	QDomElement connections;
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "units")
			{
				for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
				{
					QDomElement e = n.toElement();
					if (!e.isNull() && e.tagName() == "unit")
					{
						Unit* unit = load_unit (e.attribute ("urn"));
						if (unit)
						{
							unit->set_id (e.attribute ("id").toInt());
							Presetable* presetable = dynamic_cast<Presetable*> (unit);
							if (presetable)
								_units_to_frames_map[unit]->set_preset (e.attribute ("preset-uuid"), e.attribute ("preset-name"));
							SaveableState* saveable_state = dynamic_cast<SaveableState*> (unit);
							if (saveable_state)
								saveable_state->load_state (e);
						}
					}
				}
			}
			else if (e.tagName() == "connections")
				connections = e.toElement();
		}
	}
	// Setup connections between units:
	if (!connections.isNull())
	{
		ConnectionsDump connections_dump;
		connections_dump.insert_units (this->units().begin(), this->units().end());
		connections_dump.insert_unit (session()->audio_backend());
		connections_dump.insert_unit (session()->event_backend());
		connections_dump.load_state (connections);
		connections_dump.load();
	}
}


int
Patch::unit_tab_position (Unit* unit) const
{
	int tab_position = -1;
	UnitsToFramesMap::const_iterator f = _units_to_frames_map.find (unit);
	if (f != _units_to_frames_map.end())
		tab_position = _tabs->indexOf (f->second);
	return tab_position;
}


Q3PopupMenu*
Patch::create_units_menu()
{
	Q3PopupMenu* menu = new Q3PopupMenu (this);

	_urns.clear();

	UnitLoader::UnitFactoryList const& list = session()->unit_loader()->unit_factories();
	for (UnitLoader::UnitFactoryList::const_iterator u = list.begin(); u != list.end(); ++u)
	{
		UnitFactory::InformationMap::const_iterator title = (*u)->information().find ("haruhi:title");
		UnitFactory::InformationMap::const_iterator urn = (*u)->information().find ("haruhi:urn");
		if (title != (*u)->information().end() && urn != (*u)->information().end())
			_urns[menu->insertItem (QString::fromStdString (title->second))] = QString::fromStdString (urn->second);
	}
	QObject::connect (menu, SIGNAL (activated (int)), this, SLOT (load_unit_request (int)));

	return menu;
}


void
Patch::load_unit_request (int i)
{
	load_unit (_urns[i]);
}

} // namespace Haruhi

