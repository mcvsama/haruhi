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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__PRESET_EDITOR_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__PRESET_EDITOR_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

// Local:
#include "presets_manager.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

class PackageItem;
class CategoryItem;
class PresetItem;

class PresetEditor: public QWidget
{
	Q_OBJECT

  public:
	PresetEditor (QWidget* parent);

	void
	clear();

	void
	load_package (PackageItem* package_item);

	void
	save_package (PackageItem* package_item);

	void
	load_category (CategoryItem* category_item);

	void
	save_category (CategoryItem* category_item);

	void
	load_preset (PresetItem* preset_item);

	void
	save_preset (PresetItem* preset_item);

	void
	focus_package();

	void
	focus_category();

	void
	focus_name();

  private slots:
	void
	update_details();

  signals:
	/**
	 * Emitted when user clicks "Update details" button.
	 */
	void
	details_updated();

  private:
	QLineEdit*		_package;
	QLineEdit*		_category;
	QLineEdit*		_name;
	QLineEdit*		_version;
	QPushButton*	_update_details_button;

	PackageItem*	_package_item;
	CategoryItem*	_category_item;
	PresetItem*		_preset_item;
};

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

