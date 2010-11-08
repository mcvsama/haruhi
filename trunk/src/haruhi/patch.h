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

#ifndef HARUHI__PATCH_H__INCLUDED
#define HARUHI__PATCH_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <map>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <Qt3Support/Q3PopupMenu>

// Haruhi:
#include <haruhi/components/ports_connector/ports_connector.h>
#include <haruhi/plugin/plugin.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/unit_bay.h>


namespace Haruhi {

class Patch;
class PresetsManager;


namespace PatchPrivate {

	class ConnectionsTab: public QWidget
	{
		Q_OBJECT

	  public:
		ConnectionsTab (Patch* patch, QWidget* parent);

		~ConnectionsTab();

		PortsConnector*
		ports_connector() const { return _ports_connector; }

	  private:
		Patch*			_patch;
		PortsConnector*	_ports_connector;
	};


	class PluginTab: public QWidget
	{
		Q_OBJECT

	  public:
		PluginTab (Patch* patch, QWidget* parent, Plugin* plugin);

		~PluginTab();

		QString
		preset_uuid() const { return _preset_uuid; }

		QString
		preset_name() const { return _preset_name->text(); }

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
		Patch*			_patch;
		Plugin*			_plugin;
		QStackedWidget*	_stack;
		QCheckBox*		_favorite_checkbox;
		QString			_preset_uuid;
		QLabel*			_preset_name;
		QPushButton*	_presets_button;
		PresetsManager*	_presets_manager;
		Q3PopupMenu*	_menu;
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

	typedef std::map<Plugin*, PatchPrivate::PluginTab*> PluginsToFramesMap;

  public:
	// Ctor
	Patch (Session*, std::string const& title, QWidget* parent);

	// Dtor
	virtual ~Patch();

	Session*
	session() const { return _session; }

	Plugin*
	load_plugin (QString const&);

	void
	unload_plugin (Plugin*);

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

	/**
	 * Returns tab-position of given plugin.
	 */
	int
	plugin_tab_position (Plugin*) const;

  private:
	Q3PopupMenu*
	create_plugins_menu();

  private slots:
	void
	load_plugin_request (int i);

  private:
	Session*						_session;
	QVBoxLayout*					_layout;
	QTabWidget*						_tabs;
	PatchPrivate::ConnectionsTab*	_connections_tab;
	PluginsToFramesMap				_plugins_to_frames_map;
	Q3PopupMenu*					_plugins_menu;
	std::map<int, QString>			_urns;
};

} // namespace Haruhi

#endif

