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
#include <haruhi/plugin/plugin_factory.h>
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
	_ports_connector->add_external_unit (_patch->session()->audio_backend());
	_ports_connector->add_external_unit (_patch->session()->event_backend());

	layout->addWidget (_ports_connector);
}


ConnectionsTab::~ConnectionsTab()
{
	delete _ports_connector;
}


PluginTab::PluginTab (Patch* patch, QWidget* parent, Plugin* plugin):
	QWidget (parent),
	_patch (patch),
	_plugin (plugin)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	bool plugin_is_presetable = dynamic_cast<Presetable*> (_plugin);

	QWidget* bar = new QWidget (this);
	_stack = new QStackedWidget (this);

	_menu = new Q3PopupMenu (this);
	_menu->insertItem (Config::Icons16::remove(), "Unload", this, SLOT (unload()));

	// Title/menu button:
	QPushButton* title_button = new QPushButton (QString::fromStdString (_plugin->title()), bar);
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
	if (plugin_is_presetable)
	{
		_presets_manager = new PresetsManager (_plugin, this);
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
	if (plugin_is_presetable)
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

	_plugin->reparent (_stack, QPoint(), true);
	if (plugin_is_presetable)
		_stack->addWidget (_presets_manager);
	_stack->addWidget (_plugin);
	_stack->setCurrentWidget (_plugin);

	layout->addWidget (bar);
	layout->addWidget (_stack);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
}


PluginTab::~PluginTab()
{
	delete _menu;
}


void
PluginTab::unload()
{
	_patch->unload_plugin (_plugin);
}


void
PluginTab::presets()
{
	if (_stack->currentWidget() == _presets_manager)
	{
		_stack->setCurrentWidget (_plugin);
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
PluginTab::set_preset (QString const& uuid, QString const& name)
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
PluginTab::favorited (bool set)
{
	if (_presets_manager)
		_presets_manager->set_favorite (_preset_uuid, _preset_name->text(), set);
}

} // namespace PatchPrivate


namespace Private = PatchPrivate;


Patch::Patch (Session* session, std::string const& title, QWidget* parent):
	UnitBay (session, "", title, 0, parent),
	_session (session)
{
	// Register itself:
	_session->graph()->register_unit (this);

	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_plugins_menu = create_plugins_menu();

	QWidget* add_plugin_frame = new QWidget (this);

	QPushButton* add_plugin_button = new QPushButton (Config::Icons16::add(), "Load plugin", add_plugin_frame);
	add_plugin_button->setFlat (true);
	add_plugin_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);
	add_plugin_button->setPopup (_plugins_menu);

	QVBoxLayout* add_plugin_layout = new QVBoxLayout (add_plugin_frame, 0, 0);
	add_plugin_layout->addItem (new QSpacerItem (0, Config::spacing, QSizePolicy::Fixed, QSizePolicy::Expanding));
	add_plugin_layout->addWidget (add_plugin_button);

	_layout = new QVBoxLayout (this, 0, Config::spacing);

		_tabs = new QTabWidget (this);
		_tabs->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
		_tabs->setTabPosition (QTabWidget::South);
		_tabs->setIconSize (QSize (32, 22));
		_tabs->setMovable (true);
		_tabs->setCornerWidget (add_plugin_frame, Qt::BottomRightCorner);

			_connections_tab = new Private::ConnectionsTab (this, _tabs);

		_tabs->addTab (_connections_tab, Config::Icons22::connections(), "Connections");

	_layout->addWidget (_tabs);
}


Patch::~Patch()
{
	// Delete _connections_tab manually, to early delete PortsConnector:
	delete _connections_tab;
	delete _plugins_menu;

	while (!units().empty())
	{
		Units::iterator u = units().begin();
		// TODO use unloader for deleting objects:
		delete *u; // Deleting plugin will automatically remove it from plugins list (via graph notification).
	}
	units().clear();

	// Unregister itself:
	_session->graph()->unregister_unit (this);
}


Plugin*
Patch::load_plugin (QString const& urn)
{
	Plugin* plugin = session()->plugin_loader()->load (urn.toStdString());
	if (plugin)
	{
		// If plugin isn't already in plugins list, insert it:
		if (units().find (plugin) == units().end())
		{
			units().insert (plugin);
			_connections_tab->ports_connector()->insert_unit (plugin);
		}

		// If plugin is UnitBayAware, setup self as UnitBay:
		UnitBayAware* unit_bay_aware_plugin = dynamic_cast<UnitBayAware*> (plugin);
		if (unit_bay_aware_plugin)
			unit_bay_aware_plugin->set_unit_bay (this);

		// Create unit frame:
		Private::PluginTab* plugin_tab = new Private::PluginTab (this, _tabs, plugin);
		// TODO patch should give a hint to plugin how should it call itself in Core::Graph:
		_tabs->addTab (plugin_tab, Config::Icons22::spacer(), QString ("%1: %2").arg (1).arg (QString::fromStdString (plugin->title())));
		_plugins_to_frames_map[plugin] = plugin_tab;
	}
	return plugin;
}


void
Patch::unload_plugin (Plugin* plugin)
{
	PluginsToFramesMap::iterator u = _plugins_to_frames_map.find (plugin);
	if (u != _plugins_to_frames_map.end())
	{
		Private::PluginTab* plugin_tab = u->second;
		// Remove plugin from ports connector:
		_connections_tab->ports_connector()->remove_unit (plugin);
		// Remove plugin from plugin bay:
		units().erase (plugin);
		// Unload plugin:
		session()->plugin_loader()->unload (plugin);
		// Dispose of tab:
		delete plugin_tab;
		_plugins_to_frames_map.erase (u);
	}
}


void
Patch::save_state (QDomElement& element) const
{
	QDomElement plugins = element.ownerDocument().createElement ("plugins");

	// Tabs sorted by their tab-position:
	std::multimap<int, QDomElement> sorted_plugins;
	for (Units::const_iterator u = this->units().begin(); u != this->units().end(); ++u)
	{
		Plugin* p = dynamic_cast<Plugin*> (*u);
		if (p)
		{
			QDomElement plugin = element.ownerDocument().createElement ("plugin");
			// Plugin attributes:
			plugin.setAttribute ("urn", QString::fromStdString (p->urn()));
			plugin.setAttribute ("title", QString::fromStdString (p->title()));
			plugin.setAttribute ("id", QString ("%1").arg (p->id()));
			// Presetable?
			Presetable* presetable = dynamic_cast<Presetable*> (p);
			if (presetable)
			{
				plugin.setAttribute ("preset-name", _plugins_to_frames_map.find (p)->second->preset_name());
				plugin.setAttribute ("preset-uuid", _plugins_to_frames_map.find (p)->second->preset_uuid());
			}
			// SaveableState?
			SaveableState* saveable_state = dynamic_cast<SaveableState*> (p);
			if (saveable_state)
				saveable_state->save_state (plugin);
			// Tab position:
			sorted_plugins.insert (std::make_pair (plugin_tab_position (p), plugin));
		}
	}
	for (std::multimap<int, QDomElement>::iterator u = sorted_plugins.begin(); u != sorted_plugins.end(); ++u)
		plugins.appendChild (u->second);

	QDomElement connections = element.ownerDocument().createElement ("connections");
	ConnectionsDump connections_dump;
	connections_dump.insert_units (this->units().begin(), this->units().end());
	connections_dump.insert_unit (session()->audio_backend());
	connections_dump.insert_unit (session()->event_backend());
	connections_dump.save();
	connections_dump.save_state (connections);

	element.appendChild (plugins);
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
			if (e.tagName() == "plugins")
			{
				for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
				{
					QDomElement e = n.toElement();
					if (!e.isNull() && e.tagName() == "plugin")
					{
						Plugin* plugin = load_plugin (e.attribute ("urn"));
						if (plugin)
						{
							plugin->set_id (e.attribute ("id").toInt());
							Presetable* presetable = dynamic_cast<Presetable*> (plugin);
							if (presetable)
								_plugins_to_frames_map[plugin]->set_preset (e.attribute ("preset-uuid"), e.attribute ("preset-name"));
							SaveableState* saveable_state = dynamic_cast<SaveableState*> (plugin);
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
	// Setup connections between plugins:
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
Patch::plugin_tab_position (Plugin* plugin) const
{
	int tab_position = -1;
	PluginsToFramesMap::const_iterator f = _plugins_to_frames_map.find (plugin);
	if (f != _plugins_to_frames_map.end())
		tab_position = _tabs->indexOf (f->second);
	return tab_position;
}


Q3PopupMenu*
Patch::create_plugins_menu()
{
	Q3PopupMenu* menu = new Q3PopupMenu (this);

	_urns.clear();

	PluginLoader::PluginFactories const& list = session()->plugin_loader()->plugin_factories();
	for (PluginLoader::PluginFactories::const_iterator u = list.begin(); u != list.end(); ++u)
	{
		PluginFactory::InformationMap::const_iterator title = (*u)->information().find ("haruhi:title");
		PluginFactory::InformationMap::const_iterator urn = (*u)->information().find ("haruhi:urn");
		if (title != (*u)->information().end() && urn != (*u)->information().end())
			_urns[menu->insertItem (QString::fromStdString (title->second))] = QString::fromStdString (urn->second);
	}
	QObject::connect (menu, SIGNAL (activated (int)), this, SLOT (load_plugin_request (int)));

	return menu;
}


void
Patch::load_plugin_request (int i)
{
	load_plugin (_urns[i]);
}

} // namespace Haruhi

