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
#include <map>

// Qt:
#include <QApplication>
#include <QDragEnterEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMenu>
#include <QMimeData>
#include <QDrag>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "presets_tree.h"
#include "presets_manager.h"
#include "preset_item.h"
#include "package_item.h"
#include "category_item.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

PresetsTree::PresetsTree (PresetsManager* presets_manager, QWidget* parent):
	QTreeWidget (parent),
	_presets_manager (presets_manager)
{
	header()->setSectionsClickable (false);
	header()->setSectionResizeMode (QHeaderView::Stretch);
	header()->hide();
	sortByColumn (0, Qt::AscendingOrder);
	setMinimumWidth (160);
	setSortingEnabled (true);
	setSelectionMode (QTreeWidget::SingleSelection);
	setRootIsDecorated (true);
	setAllColumnsShowFocus (true);
	setAcceptDrops (true);
	setAutoScroll (true);
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
	setContextMenuPolicy (Qt::CustomContextMenu);
	setIconSize (Resources::Icons16::haruhi().size());
	setHeaderLabel ("Presets");

	_auto_open_timer = std::make_unique<QTimer> (this);
	_auto_open_timer->setSingleShot (true);
	QObject::connect (_auto_open_timer.get(), SIGNAL (timeout()), this, SLOT (auto_open_selected()));
	QObject::connect (this, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu (const QPoint&)));
}


PresetsTree::~PresetsTree()
{
	_auto_open_timer->stop();
}


QTreeWidgetItem*
PresetsTree::selected_item() const
{
	auto list = selectedItems();
	return list.empty() ? 0 : list.front();
}


PackageItem*
PresetsTree::current_package_item() const
{
	if (auto item = selected_item())
		return dynamic_cast<PackageItem*> (item);
	return nullptr;
}


CategoryItem*
PresetsTree::current_category_item() const
{
	if (auto item = selected_item())
		return dynamic_cast<CategoryItem*> (item);
	return nullptr;
}


PresetItem*
PresetsTree::current_preset_item() const
{
	if (auto item = selected_item())
		return dynamic_cast<PresetItem*> (item);
	return nullptr;
}


void
PresetsTree::auto_open_selected()
{
	if (auto item = selected_item())
		item->setExpanded (true);
}


void
PresetsTree::context_menu (QPoint const& pos)
{
	auto item = itemAt (pos);
	auto menu = std::make_unique<QMenu> (this);

	if (!item)
		menu->addAction (Resources::Icons16::presets_package(), "New pac&kage", _presets_manager, SLOT (create_package()));
	else
	{
		item->setSelected (true);

		if (dynamic_cast<PackageItem*> (item))
		{
			menu->addAction (Resources::Icons16::presets_category(), "New &category", _presets_manager, SLOT (create_category()));
			menu->addAction (Resources::Icons16::presets_package(), "New pac&kage", _presets_manager, SLOT (create_package()));
			menu->addSeparator();
			menu->addAction (Resources::Icons16::remove(), "Destroy package", _presets_manager, SLOT (destroy()));
		}
		else if (dynamic_cast<CategoryItem*> (item))
		{
			menu->addAction (Resources::Icons16::preset(), "New &preset", _presets_manager, SLOT (create_preset()));
			menu->addAction (Resources::Icons16::presets_category(), "New &category", _presets_manager, SLOT (create_category()));
			menu->addAction (Resources::Icons16::presets_package(), "New pac&kage", _presets_manager, SLOT (create_package()));
			menu->addSeparator();
			menu->addAction (Resources::Icons16::remove(), "Destroy category", _presets_manager, SLOT (destroy()));
		}
		else if (dynamic_cast<PresetItem*> (item))
		{
			menu->addAction (Resources::Icons16::load(), "&Load preset", _presets_manager, SLOT (load_preset()));
			menu->addSeparator();
			menu->addAction (Resources::Icons16::preset(), "New &preset", _presets_manager, SLOT (create_preset()));
			menu->addAction (Resources::Icons16::presets_category(), "New &category", _presets_manager, SLOT (create_category()));
			menu->addAction (Resources::Icons16::presets_package(), "New pac&kage", _presets_manager, SLOT (create_package()));
			menu->addSeparator();
			menu->addAction (Resources::Icons16::remove(), "Destroy preset", _presets_manager, SLOT (destroy()));
		}
	}

	menu->exec (QCursor::pos());
}


void
PresetsTree::dragEnterEvent (QDragEnterEvent* event)
{
	PresetsTree* source;
	if (event->source() && (source = dynamic_cast<PresetsTree*> (event->source())) &&
		source == this && event->mimeData()->hasText())
	{
		event->accept();
	}
	else
		event->ignore();
}


void
PresetsTree::dragMoveEvent (QDragMoveEvent* event)
{
	PresetsTree* source;
	if (event->source() && (source = dynamic_cast<PresetsTree*> (event->source())) &&
		source == this && event->mimeData()->hasText())
	{
		auto to = drag_drop_item (event->pos());
		if (to && can_drop (_dragged_item, to))
			event->accept();
		else
			event->ignore();
	}
	else
		event->ignore();
}


void
PresetsTree::dragLeaveEvent (QDragLeaveEvent*)
{
	_dropped_on_item = nullptr;
}


void
PresetsTree::dropEvent (QDropEvent* event)
{
	PresetsTree* source;
	if (event->source() && (source = dynamic_cast<PresetsTree*> (event->source())) &&
		source == this && event->mimeData()->hasText())
	{
		auto to = drag_drop_item (event->pos());
		if (_dragged_item && to)
		{
			PackageItem* package_item;
			CategoryItem* category_item;
			PresetItem* preset_item;

			// Move Preset to Category:
			if ((preset_item = dynamic_cast<PresetItem*> (_dragged_item)) && (category_item = dynamic_cast<CategoryItem*> (to)))
			{
				auto old_category_item = preset_item->category_item();
				auto new_category_item = category_item;
				old_category_item->removeChild (preset_item);
				new_category_item->addChild (preset_item);
				new_category_item->setExpanded (true);
				preset_item->treeWidget()->clearSelection();
				preset_item->setSelected (true);
				_presets_manager->model()->save_state();
			}
			// Move Category to Package:
			else if ((category_item = dynamic_cast<CategoryItem*> (_dragged_item)) && (package_item = dynamic_cast<PackageItem*> (to)))
			{
				auto old_package_item = category_item->package_item();
				auto new_package_item = package_item;
				old_package_item->removeChild (category_item);
				new_package_item->addChild (category_item);
				new_package_item->setExpanded (true);
				category_item->treeWidget()->clearSelection();
				category_item->setSelected (true);
				_presets_manager->model()->save_state();
			}
		}
	}
	event->acceptProposedAction();
	dragLeaveEvent (0);
}


void
PresetsTree::mousePressEvent (QMouseEvent* mouse_event)
{
	QTreeWidget::mousePressEvent (mouse_event);

	if (mouse_event->button() == Qt::LeftButton)
	{
		_drag_pos = mouse_event->pos();
		auto item = itemAt (_drag_pos);
		if (item && item->flags() & Qt::ItemIsDragEnabled)
			_dragged_item = itemAt (_drag_pos);
	}
}


void
PresetsTree::mouseMoveEvent (QMouseEvent* mouse_event)
{
	QTreeWidget::mouseMoveEvent (mouse_event);

	if ((mouse_event->buttons() & Qt::LeftButton) && _dragged_item &&
		((mouse_event->pos() - _drag_pos).manhattanLength() >= QApplication::startDragDistance()))
	{
		auto mime_data = new QMimeData();
		mime_data->setText (_dragged_item->text (0));
		auto drag = new QDrag (this);
		drag->setMimeData (mime_data);
		drag->setPixmap (_dragged_item->icon (0).pixmap (16));
		drag->setHotSpot (QPoint (-4, -12));
		drag->start (Qt::LinkAction);
		// We've dragged and maybe dropped it by now...
		_dragged_item = 0;
	}
}


bool
PresetsTree::can_drop (QTreeWidgetItem* from, QTreeWidgetItem* to)
{
	return (dynamic_cast<PresetItem*> (from) && dynamic_cast<CategoryItem*> (to) && from->parent() != to)
		|| (dynamic_cast<CategoryItem*> (from) && dynamic_cast<PackageItem*> (to) && from->parent() != to);
}


QTreeWidgetItem*
PresetsTree::drag_drop_item (QPoint const& epos)
{
	auto item = itemAt (epos);
	if (item)
	{
		if (item != _dropped_on_item)
		{
			clearSelection();
			item->setSelected (true);
			_dropped_on_item = item;
			_auto_open_timer->start (750);
		}
	}
	else
	{
		_dropped_on_item = 0;
		_auto_open_timer->stop();
	}
	return item;
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

