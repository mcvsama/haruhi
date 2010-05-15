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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__PRESETS_LIST_VIEW_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__PRESETS_LIST_VIEW_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QTimer>
#include <Qt3Support/Q3DragObject>
#include <Qt3Support/Q3ListView>

// Local:
#include "presets_manager.h"


namespace Haruhi {

class PresetsManager;

namespace PresetsManagerPrivate {

class PackageItem;
class CategoryItem;
class PresetItem;

class PresetsListView: public Q3ListView
{
	Q_OBJECT

  public:
	PresetsListView (PresetsManager* manager, QWidget* parent);

	~PresetsListView();

	PackageItem*
	current_package_item() const;

	CategoryItem*
	current_category_item() const;

	PresetItem*
	current_preset_item() const;

	PresetsManager*
	presets_manager() const { return _presets_manager; }

  private slots:
	void
	auto_open_selected();

	void
	context_menu (Q3ListViewItem* item, QPoint const& pos, int col);

  protected:
	/**
	 * Drag and drop methods.
	 */

	void
	dragEnterEvent (QDragEnterEvent*);

	void
	dragMoveEvent (QDragMoveEvent*);

	void
	dragLeaveEvent (QDragLeaveEvent*);

	void
	dropEvent (QDropEvent*);

	bool
	can_drop (Q3ListViewItem* from, Q3ListViewItem* to);

	Q3DragObject*
	dragObject();

	Q3ListViewItem*
	drag_drop_item (QPoint const& epos);

  private:
	PresetsManager*	_presets_manager;
	Q3ListViewItem*	_dragged_item;
	Q3ListViewItem*	_dropped_on_item;
	QTimer*			_auto_open_timer;
};

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

