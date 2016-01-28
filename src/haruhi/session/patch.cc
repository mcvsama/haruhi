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
#include <algorithm>
#include <map>

// Qt:
#include <QSignalMapper>
#include <QApplication>
#include <QPushButton>
#include <QCheckBox>
#include <QMenu>
#include <QLayout>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/graph/conn_set.h>
#include <haruhi/plugin/plugin_factory.h>
#include <haruhi/plugin/has_presets.h>
#include <haruhi/session/session.h>
#include <haruhi/components/presets_manager/presets_manager.h>
#include <haruhi/widgets/texture_widget.h>
#include <haruhi/utility/qdom.h>

// Local:
#include "patch.h"


namespace Haruhi {

namespace PatchPrivate {

ConnectionsTab::ConnectionsTab (Patch* patch, QWidget* parent):
	QWidget (parent),
	_patch (patch)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setContentsMargins (Config::margin(), Config::margin(), Config::margin(), Config::margin());

	_ports_connector = std::make_unique<PortsConnector> (_patch, this);
	_ports_connector->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_ports_connector->add_external_unit (_patch->session()->graph()->audio_backend());
	_ports_connector->add_external_unit (_patch->session()->graph()->event_backend());

	auto top_layout = new QHBoxLayout();
	top_layout->setSpacing (Config::spacing());
	top_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	auto layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addLayout (top_layout);
	layout->addWidget (_ports_connector.get());
}


PluginTab::PluginTab (Patch* patch, QWidget* parent, Plugin* plugin):
	QWidget (parent),
	_patch (patch),
	_plugin (plugin)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	bool plugin_is_has_presets = dynamic_cast<HasPresets*> (_plugin);

	auto bar = new QWidget (this);
	_stack = std::make_unique<QStackedWidget> (this);

	_menu = std::make_unique<QMenu> (this);
	_menu->addAction (Resources::Icons16::remove(), "Unload", this, SLOT (unload()));

	// Title/menu button:
	auto title_button = new QPushButton (QString::fromStdString (_plugin->title()), bar);
	title_button->setIconSize (Resources::Icons16::haruhi().size());
	title_button->clearFocus();
	title_button->setFlat (true);
	title_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	title_button->setMenu (_menu.get());
	QPalette p = title_button->palette();
	p.setColor (QPalette::Button, QColor (0x00, 0x2A, 0x5B));
	p.setColor (QPalette::ButtonText, QColor (0xff, 0xff, 0xff));
	title_button->setPalette (p);
	QFont font (QApplication::font());
	font.setWeight (QFont::Black);
	title_button->setFont (font);

	// HasPresets?
	if (plugin_is_has_presets)
	{
		_presets_manager = std::make_unique<PresetsManager> (_plugin, this);
		_presets_manager->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
		QObject::connect (_presets_manager.get(), SIGNAL (preset_selected (const QString&, const QString&)),
						  this, SLOT (set_preset (const QString&, const QString&)));

		// Preset label:
		_preset_name = std::make_unique<QLabel> ("", bar);
		QFont font (QApplication::font());
		font.setWeight (QFont::Black);
		_preset_name->setFont (font);
		_preset_name->setTextFormat (Qt::PlainText);

		// Presets button:
		_presets_button = std::make_unique<QPushButton> ("Presets", bar);
		_presets_button->setIconSize (Resources::Icons16::haruhi().size());
		_presets_button->clearFocus();
		_presets_button->setFlat (true);
		_presets_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		QPalette p = _presets_button->palette();
		p.setColor (QPalette::Button, QColor (0x00, 0x2A, 0x5B));
		p.setColor (QPalette::ButtonText, QColor (0xff, 0xff, 0xff));
		_presets_button->setPalette (p);
		QObject::connect (_presets_button.get(), SIGNAL (clicked()), this, SLOT (presets()));

		// Presets manager/favoriting:

		_favorite_button = std::make_unique<QPushButton> ("Favorite", bar);
		_favorite_button->setIconSize (Resources::Icons16::haruhi().size());
		_favorite_button->clearFocus();
		_favorite_button->setFlat (true);
		_favorite_button->setCheckable (true);
		_favorite_button->setEnabled (false);
		QObject::connect (_favorite_button.get(), SIGNAL (toggled (bool)), this, SLOT (favorited (bool)));
	}

	p = bar->palette();
	p.setColor (QPalette::Window, QColor (0x00, 0x2A, 0x5B));
	p.setColor (QPalette::WindowText, Qt::white);
	bar->setPalette (p);
	bar->setAutoFillBackground (true);

	_plugin_container = std::make_unique<QWidget> (this);
	auto plugin_with_background = new TextureWidget (TextureWidget::Filling::Solid, this);
	plugin_with_background->setSizePolicy (_plugin->sizePolicy());

	auto plugin_with_background_layout = new QGridLayout (plugin_with_background);
	plugin_with_background_layout->setMargin (0);
	plugin_with_background_layout->setSpacing (0);
	plugin_with_background_layout->addWidget (_plugin, 0, 0);

	auto plugin_container_layout = new QGridLayout (_plugin_container.get());
	plugin_container_layout->setMargin (0);
	plugin_container_layout->setSpacing (0);
	plugin_container_layout->addWidget (new TextureWidget (TextureWidget::Filling::Dotted, this), 0, 0, 2, 2);
	plugin_container_layout->addWidget (plugin_with_background, 0, 0, 1, 1);

	_plugin->setParent (plugin_with_background);
	_plugin->show();
	if (plugin_is_has_presets)
		_stack->addWidget (_presets_manager.get());
	_stack->addWidget (_plugin_container.get());
	_stack->setCurrentWidget (_plugin_container.get());

	// Layouts:

	auto bar_layout = new QHBoxLayout (bar);
	bar_layout->setMargin (0);
	bar_layout->setSpacing (Config::spacing());
	bar_layout->addWidget (title_button);
	if (plugin_is_has_presets)
	{
		bar_layout->addItem (new QSpacerItem (5, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
		bar_layout->addWidget (_preset_name.get());
		bar_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
		bar_layout->addWidget (_favorite_button.get());
		bar_layout->addItem (new QSpacerItem (10, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
		bar_layout->addWidget (_presets_button.get());
	}
	else
		bar_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	auto layout = new QVBoxLayout (this);
	layout->setMargin (Config::margin());
	layout->setSpacing (Config::spacing());
	layout->addWidget (bar);
	layout->addWidget (_stack.get());

	update_widgets();
}


PluginTab::~PluginTab()
{ }


void
PluginTab::unload()
{
	// Do it asynchronously, since QMenu that implements the "unload" action
	// is inside the plugin, so deleting plugin might be synchronous and
	// delete object that's still in use (QMenu).
	_patch->unload_plugin_later (_plugin);
}


void
PluginTab::presets()
{
	if (_stack->currentWidget() == _presets_manager.get())
	{
		_stack->setCurrentWidget (_plugin_container.get());
		QPalette p = _presets_button->palette();
		p.setColor (QPalette::Button, QColor (0x00, 0x2A, 0x5B));
		p.setColor (QPalette::ButtonText, QColor (0xff, 0xff, 0xff));
		_presets_button->setPalette (p);
	}
	else
	{
		_stack->setCurrentWidget (_presets_manager.get());
		QPalette p = _presets_button->palette();
		p.setColor (QPalette::Button, QColor (0x0c, 0x62, 0xf8));
		p.setColor (QPalette::ButtonText, QColor (0xff, 0xff, 0xff));
		_presets_button->setPalette (p);
	}

	_presets_button->setAutoFillBackground (true);
}


void
PluginTab::set_preset (QString const& uuid, QString const& name)
{
	if (_presets_manager)
	{
		_preset_uuid = uuid;
		_preset_name->setText (name);
		_favorite_button->setEnabled (!uuid.isNull());
		_favorite_button->setChecked (_presets_manager->favorited (uuid));
		update_widgets();
	}
}


void
PluginTab::favorited (bool set)
{
	if (_presets_manager)
	{
		_presets_manager->set_favorited (_preset_uuid, set);
		update_widgets();
	}
}


void
PluginTab::update_widgets()
{
	if (dynamic_cast<HasPresets*> (_plugin))
	{
		QPalette p = _favorite_button->palette();
		p.setColor (QPalette::Button, _favorite_button->isChecked() ? QColor (0x00, 0xff, 0x00) : QColor (0x00, 0x2A, 0x5B));
		p.setColor (QPalette::ButtonText, Qt::white);
		_favorite_button->setPalette (p);
		_favorite_button->setAutoFillBackground (false);
	}
}

} // namespace PatchPrivate


namespace Private = PatchPrivate;

using namespace ScreenLiterals;


Patch::Patch (Session* session, std::string const& title, QWidget* parent):
	UnitBay ("", title, 0x20000, parent),
	_session (session)
{
	// Register itself:
	_session->graph()->register_unit (this);

	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	create_plugins_menu();

	auto add_plugin_frame = new QWidget (this);

	auto add_plugin_button = new QPushButton (Resources::Icons16::add(), "Load plugin", add_plugin_frame);
	add_plugin_button->setIconSize (Resources::Icons16::haruhi().size());
	add_plugin_button->setFlat (true);
	add_plugin_button->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);
	add_plugin_button->setMenu (_plugins_menu.get());

	_tabs = std::make_unique<QTabWidget> (this);
	_tabs->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_tabs->setTabPosition (QTabWidget::North);
	_tabs->setIconSize (Resources::Icons16::haruhi().size() * 1.5);
	_tabs->setMovable (true);
	_tabs->setCornerWidget (add_plugin_frame, Qt::BottomRightCorner);

	_connections_tab = std::make_unique<Private::ConnectionsTab> (this, _tabs.get());

	_tabs->addTab (_connections_tab.get(), Resources::Icons16::connections(), "Wires");
	_tabs->setIconSize (Resources::Icons16::haruhi().size() * 1.5);

	// Layouts:

	auto add_plugin_layout = new QVBoxLayout (add_plugin_frame);
	add_plugin_layout->setMargin (0);
	add_plugin_layout->setSpacing (0);
	add_plugin_layout->addWidget (add_plugin_button);
	add_plugin_layout->addItem (new QSpacerItem (0, Config::spacing(), QSizePolicy::Fixed, QSizePolicy::Expanding));

	auto layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (Config::spacing());
	layout->addWidget (_tabs.get());
}


Patch::~Patch()
{
	// Need to early delete ConnectionsTab along with PortsConnector:
	_connections_tab.reset();

	while (!units().empty())
	{
		Units::iterator u = units().begin();
		if (auto plugin = dynamic_cast<Plugin*> (*u))
			unload_plugin (plugin);
	}
	units().clear();

	// Unregister itself:
	disable();
	_session->graph()->unregister_unit (this);
}


Plugin*
Patch::load_plugin (QString const& urn)
{
	auto plugin = session()->plugin_loader()->load (urn.toStdString());

	if (plugin)
	{
		// If plugin isn't already in plugins list, insert it:
		if (units().find (plugin) == units().end())
		{
			units().insert (plugin);
			_connections_tab->ports_connector()->insert_unit (plugin);
		}

		// If plugin is UnitBayAware, setup self as UnitBay:
		if (auto unit_bay_aware_plugin = dynamic_cast<UnitBayAware*> (plugin))
			unit_bay_aware_plugin->set_unit_bay (this);

		// Create unit frame:
		auto plugin_tab = std::make_unique<Private::PluginTab> (this, _tabs.get(), plugin);
		_tabs->addTab (plugin_tab.get(), Resources::Icons16::unit_type_synth(), "<>"); // TODO Icon should depend on plugin's declared type
		_plugins_to_frames_map[plugin] = std::move (plugin_tab);
		update_tab_title (plugin);
		_session->graph()->register_unit (plugin);
	}

	return plugin;
}


void
Patch::unload_plugin (Plugin* plugin)
{
	// Unregister plugin:
	_session->graph()->unregister_unit (plugin);

	PluginsToFramesMap::iterator u = _plugins_to_frames_map.find (plugin);

	if (u != _plugins_to_frames_map.end())
	{
		// Remove plugin from plugin bay:
		units().erase (plugin);
		// Unload plugin:
		session()->plugin_loader()->unload (plugin);
		// Dispose of tab:
		_plugins_to_frames_map.erase (u);
	}
}


void
Patch::unload_plugin_later (Plugin* plugin)
{
	// TODO prevent double unregistration (by disabling QMenu action?):
	auto callback = [=](std::weak_ptr<int> tracker) {
		if (tracker.lock())
			unload_plugin (plugin);
	};

	Services::call_out (std::bind (callback, std::weak_ptr<int> (_lifetime_tracker)));
}


int
Patch::plugin_tab_position (Plugin* plugin) const
{
	int tab_position = -1;
	PluginsToFramesMap::const_iterator f = _plugins_to_frames_map.find (plugin);
	if (f != _plugins_to_frames_map.end())
		tab_position = _tabs->indexOf (f->second.get());
	return tab_position;
}


void
Patch::update_tab_title (Plugin* plugin)
{
	int i = plugin_tab_position (plugin);
	if (i != -1)
		_tabs->setTabText (i, QString ("%1: %2").arg (plugin->id()).arg (QString::fromStdString (plugin->title())));
}


void
Patch::show_prev_tab()
{
	int c = (_tabs->currentIndex() - 1) % _tabs->count();
	if (c < 0)
		c = _tabs->count() - 1;
	_tabs->setCurrentIndex (c);
}


void
Patch::show_next_tab()
{
	_tabs->setCurrentIndex ((_tabs->currentIndex() + 1) % _tabs->count());
}


void
Patch::save_state (QDomElement& element) const
{
	QDomElement plugins = element.ownerDocument().createElement ("plugins");

	// Tabs sorted by their tab-position:
	std::multimap<int, QDomElement> sorted_plugins;
	for (Unit* u: units())
	{
		if (auto p = dynamic_cast<Plugin*> (u))
		{
			QDomElement plugin = element.ownerDocument().createElement ("plugin");
			// Plugin attributes:
			plugin.setAttribute ("urn", QString::fromStdString (p->urn()));
			plugin.setAttribute ("title", QString::fromStdString (p->title()));
			plugin.setAttribute ("id", QString ("%1").arg (p->id()));
			// HasPresets?
			if (dynamic_cast<HasPresets*> (p))
			{
				plugin.setAttribute ("preset-name", _plugins_to_frames_map.find (p)->second->preset_name());
				plugin.setAttribute ("preset-uuid", _plugins_to_frames_map.find (p)->second->preset_uuid());
			}
			// SaveableState?
			if (auto saveable_state = dynamic_cast<SaveableState*> (p))
				saveable_state->save_state (plugin);
			// Tab position:
			sorted_plugins.insert (std::make_pair (plugin_tab_position (p), plugin));
		}
	}

	for (auto u: sorted_plugins)
		plugins.appendChild (u.second);

	QDomElement connections = element.ownerDocument().createElement ("connections");
	ConnSet conn_set;
	graph()->synchronize ([&] {
		conn_set.insert_units (this->units().begin(), this->units().end());
		conn_set.insert_unit (session()->graph()->audio_backend());
		conn_set.insert_unit (session()->graph()->event_backend());
		conn_set.save();
		conn_set.save_state (connections);
	});

	element.appendChild (plugins);
	element.appendChild (connections);
}


void
Patch::load_state (QDomElement const& element)
{
	QDomElement connections;
	for (QDomElement& e: element)
	{
		if (e.tagName() == "plugins")
		{
			for (QDomElement& e2: e)
			{
				if (e2.tagName() == "plugin")
				{
					if (auto plugin = load_plugin (e2.attribute ("urn")))
					{
						plugin->set_id (e2.attribute ("id").toInt());
						// ID has changed, so update tab title to reflect correct ID:
						update_tab_title (plugin);

						if (dynamic_cast<HasPresets*> (plugin))
							_plugins_to_frames_map[plugin]->set_preset (e2.attribute ("preset-uuid"), e2.attribute ("preset-name"));

						if (auto saveable_state = dynamic_cast<SaveableState*> (plugin))
							saveable_state->load_state (e2);
					}
				}
			}
		}
		else if (e.tagName() == "connections")
			connections = e.toElement();
	}
	// Setup connections between plugins:
	if (!connections.isNull())
	{
		ConnSet conn_set;
		Mutex::Lock lock (*graph());
		conn_set.insert_units (this->units().begin(), this->units().end());
		conn_set.insert_unit (session()->graph()->audio_backend());
		conn_set.insert_unit (session()->graph()->event_backend());
		conn_set.load_state (connections);
		conn_set.load();
	}
}


void
Patch::create_plugins_menu()
{
	_plugins_menu = std::make_unique<QMenu> (this);
	_plugins_mapper = std::make_unique<QSignalMapper> (this);

	_urns.clear();

	auto const& list = session()->plugin_loader()->plugin_factories();
	int action_id = 0;

	for (auto& pf: list)
	{
		action_id += 1;
		auto action = _plugins_menu->addAction (QString::fromStdString (pf->title()), _plugins_mapper.get(), SLOT (map()));
		_plugins_mapper->setMapping (action, action_id);
		_urns[action_id] = QString::fromStdString (pf->urn());
	}

	QObject::connect (_plugins_mapper.get(), SIGNAL (mapped (int)), this, SLOT (load_plugin_request (int)));
}


void
Patch::load_plugin_request (int i)
{
	Plugin* plugin = load_plugin (_urns[i]);
	_tabs->setCurrentWidget (_plugins_to_frames_map[plugin].get());
}

} // namespace Haruhi

