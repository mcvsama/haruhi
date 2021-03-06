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

#ifndef HARUHI__COMPONENTS__PRESETS_MANAGER__PRESETS_TREE_H__INCLUDED
#define HARUHI__COMPONENTS__PRESETS_MANAGER__PRESETS_TREE_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QTimer>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "presets_manager.h"


namespace Haruhi {

class PresetsManager;

namespace PresetsManagerPrivate {

class PackageItem;
class CategoryItem;
class PresetItem;

class PresetsTree: public QTreeWidget
{
	Q_OBJECT

  public:
	PresetsTree (PresetsManager* manager, QWidget* parent);

	~PresetsTree();

	QTreeWidgetItem*
	selected_item() const;

	PackageItem*
	current_package_item() const;

	CategoryItem*
	current_category_item() const;

	PresetItem*
	current_preset_item() const;

	PresetsManager*
	presets_manager() const;

  private slots:
	void
	auto_open_selected();

	void
	context_menu (QPoint const& pos);

  protected:
	/**
	 * Drag and drop methods.
	 */

	void
	dragEnterEvent (QDragEnterEvent*) override;

	void
	dragMoveEvent (QDragMoveEvent*) override;

	void
	dragLeaveEvent (QDragLeaveEvent*) override;

	void
	dropEvent (QDropEvent*) override;

	void
	mousePressEvent (QMouseEvent*) override;

	void
	mouseMoveEvent (QMouseEvent*) override;

	bool
	can_drop (QTreeWidgetItem* from, QTreeWidgetItem* to);

	QTreeWidgetItem*
	drag_drop_item (QPoint const& epos);

  private:
	PresetsManager*		_presets_manager;
	QPoint				_drag_pos;			// Drag init point.
	QTreeWidgetItem*	_dragged_item		= nullptr;
	QTreeWidgetItem*	_dropped_on_item	= nullptr;
	Unique<QTimer>		_auto_open_timer;
};


inline PresetsManager*
PresetsTree::presets_manager() const
{
	return _presets_manager;
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

#endif

