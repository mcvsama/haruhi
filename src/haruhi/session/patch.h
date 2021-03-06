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

#ifndef HARUHI__SESSION__PATCH_H__INCLUDED
#define HARUHI__SESSION__PATCH_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>

// Qt:
#include <QSignalMapper>
#include <QWidget>
#include <QTabWidget>
#include <QStackedWidget>
#include <QFrame>
#include <QLabel>
#include <QCheckBox>
#include <QMenu>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/components/ports_connector/ports_connector.h>
#include <haruhi/plugin/plugin.h>
#include <haruhi/session/unit_bay.h>
#include <haruhi/utility/saveable_state.h>


namespace Haruhi {

class Patch;
class PresetsManager;
class Session;

namespace PatchPrivate {

	class ConnectionsTab: public QWidget
	{
		Q_OBJECT

	  public:
		ConnectionsTab (Patch* patch, QWidget* parent);

		PortsConnector*
		ports_connector() const;

	  private:
		Patch*					_patch;
		Unique<PortsConnector>	_ports_connector;
	};


	class PluginTab: public QWidget
	{
		Q_OBJECT

	  public:
		PluginTab (Patch* patch, QWidget* parent, Plugin* plugin);

		~PluginTab();

		QString
		preset_uuid() const;

		QString
		preset_name() const;

	  public slots:
		void
		unload();

		void
		presets();

		void
		set_preset (QString const& uuid, QString const& name);

		void
		favorited (bool);

	  private:
		void
		update_widgets();

	  private:
		Patch*					_patch;
		Plugin*					_plugin;

		// In this order (_stack also owns _plugin_container and _presets_manager):
		Unique<QStackedWidget>	_stack;
		Unique<PresetsManager>	_presets_manager;
		Unique<QWidget>			_plugin_container;

		Unique<QPushButton>		_favorite_button;
		QString					_preset_uuid;
		Unique<QLabel>			_preset_name;
		Unique<QPushButton>		_presets_button;
		Unique<QMenu>			_menu;
	};

} // namespace PatchPrivate


/**
 * Maintains a set of living Plugins and a list of connections between them.
 * Loads/saves patches.
 *
 * Warning: When restoring Plugin no graph locking is done by the Patch.
 * Plugin must ensure itself that processing-round will not
 * interfere with it, either by disabling itself for a while
 * (best option), or locking graph (xrun generator).
 */
class Patch:
	public QWidget,
	public UnitBay,
	public SaveableState
{
	Q_OBJECT

	friend class PatchPrivate::PluginTab;

	typedef std::map<Plugin*, Unique<PatchPrivate::PluginTab>> PluginsToFramesMap;

  public:
	// Ctor
	Patch (Session*, std::string const& title, QWidget* parent);

	// Dtor
	virtual ~Patch();

	Session*
	session() const;

	Plugin*
	load_plugin (QString const&);

	void
	unload_plugin (Plugin*);

	/**
	 * Call unload_plugin() from the event loop.
	 */
	void
	unload_plugin_later (Plugin*);

	/**
	 * Returns tab-position of given plugin.
	 */
	int
	plugin_tab_position (Plugin*) const;

	void
	update_tab_title (Plugin*);

	/**
	 * Switch to previous tab.
	 */
	void
	show_prev_tab();

	/**
	 * Switch to next tab.
	 */
	void
	show_next_tab();

	/*
	 * SaveableState API
	 */

	void
	save_state (QDomElement&) const override;

	void
	load_state (QDomElement const&) override;

  private:
	/**
	 * Creates _plugins_menu. May be called only once.
	 */
	void
	create_plugins_menu();

  private slots:
	void
	load_plugin_request (int i);

  private:
	std::shared_ptr<int>					_lifetime_tracker = std::make_shared<int> (0);
	Session*								_session;
	PluginsToFramesMap						_plugins_to_frames_map;
	std::map<int, QString>					_urns;
	// In this order:
	Unique<QTabWidget>						_tabs;
	Unique<QMenu>							_plugins_menu;
	Unique<QSignalMapper>					_plugins_mapper;
	Unique<PatchPrivate::ConnectionsTab>	_connections_tab;
};


inline PortsConnector*
PatchPrivate::ConnectionsTab::ports_connector() const
{
	return _ports_connector.get();
}


inline QString
PatchPrivate::PluginTab::preset_uuid() const
{
	return _preset_uuid;
}


inline QString
PatchPrivate::PluginTab::preset_name() const
{
	return _preset_name->text();
}


inline Session*
Patch::session() const
{
	return _session;
}

} // namespace Haruhi

#endif

