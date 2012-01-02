/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__PRESETS_MANAGER_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__PRESETS_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>
#include <list>

// Qt:
#include <QtGui/QPushButton>
#include <QtGui/QMenu>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/graph/unit.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/utility/signal.h>
#include <haruhi/settings/has_presets_settings.h>

// Local:
#include "model.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

class PackageItem;
class PresetEditor;
class PresetItem;
class PresetsTree;
class Package;

}


class PresetsManager:
	public QWidget,
	public Signal::Receiver
{
	Q_OBJECT

	friend class PresetsManagerPrivate::PresetsTree;

  public:
	PresetsManager (Unit*, QWidget* parent = 0);

	~PresetsManager();

	/**
	 * Model accessor.
	 */
	PresetsManagerPrivate::Model*
	model();

	/**
	 * Returns true if preset with given UUID is favorited.
	 * Searches only presets for the Unit.
	 */
	bool
	favorited (QString const& preset_uuid) const;

	/**
	 * Sets preset as favorited/not favorited.
	 */
	void
	set_favorited (QString const& preset_uuid, bool set);

  signals:
	void
	preset_selected (QString const& preset_uuid, QString const& preset_name);

  private slots:
	/**
	 * Loads Preset currently selected in list.
	 */
	void
	load_preset();

 	void
	load_preset (QTreeWidgetItem*);

	/**
	 * Save current params as selected preset.
	 */
	void
	save_preset();

	void
	create_package();

	void
	create_category();

	void
	create_preset();

	void
	destroy();

	/**
	 * Updates widgets (enabled, etc).
	 */
	void
	update_widgets();

	void
	show_favorites();

  private:
	void
	read();

	void
	save_preset (PresetsManagerPrivate::PresetItem* preset_item, bool with_patch);

	PresetsManagerPrivate::PackageItem*
	create_package_item (PresetsManagerPrivate::Package* package);

	void
	remove_package_item (PresetsManagerPrivate::PackageItem* package_item);

	std::string
	sanitize_urn (std::string const& urn) const;

  private:
	PresetsManagerPrivate::Model*			_model;
	Unit*									_unit;
	SaveableState*							_saveable_unit;
	PresetsManagerPrivate::PresetsTree*		_tree;
	PresetsManagerPrivate::PresetEditor*	_editor;
	HasPresetsSettings*						_has_presets_settings;
	QPushButton*							_only_favs_button;
	QPushButton*							_load_button;
	QPushButton*							_save_button;
	QPushButton*							_create_button;
	QPushButton*							_destroy_button;
	QMenu*									_create_menu;
	QAction*								_create_package_action;
	QAction*								_create_category_action;
	QAction*								_create_preset_action;
};


inline PresetsManagerPrivate::Model*
PresetsManager::model()
{
	return _model;
}

} // namespace Haruhi

#endif

