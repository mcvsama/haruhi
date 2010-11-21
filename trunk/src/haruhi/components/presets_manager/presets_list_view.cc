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
#include <map>

// Qt:
#include <QtGui/QApplication>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QDragEnterEvent>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "presets_list_view.h"
#include "presets_manager.h"
#include "preset_item.h"
#include "package_item.h"
#include "category_item.h"


namespace Haruhi {

namespace PresetsManagerPrivate {

PresetsListView::PresetsListView (PresetsManager* presets_manager, QWidget* parent):
	QTreeWidget (parent),
	_presets_manager (presets_manager),
	_dragged_item (0),
	_dropped_on_item (0)
{
	header()->setClickable (false);
	header()->setResizeMode (QHeaderView::Stretch);
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
	setHeaderLabel ("Presets");

	_auto_open_timer = new QTimer (this);
	QObject::connect (_auto_open_timer, SIGNAL (timeout()), this, SLOT (auto_open_selected()));
	QObject::connect (this, SIGNAL (customContextMenuRequested (const QPoint&)), this, SLOT (context_menu (const QPoint&)));
}


PresetsListView::~PresetsListView()
{
	_auto_open_timer->stop();
}


QTreeWidgetItem*
PresetsListView::selected_item() const
{
	QList<QTreeWidgetItem*> list = selectedItems();
	return list.empty() ? 0 : list.front();
}


PackageItem*
PresetsListView::current_package_item() const
{
	QTreeWidgetItem* item = selected_item();
	if (item)
		return dynamic_cast<PackageItem*> (item);
	return 0;
}


CategoryItem*
PresetsListView::current_category_item() const
{
	QTreeWidgetItem* item = selected_item();
	if (item)
		return dynamic_cast<CategoryItem*> (item);
	return 0;
}


PresetItem*
PresetsListView::current_preset_item() const
{
	QTreeWidgetItem* item = selected_item();
	if (item)
		return dynamic_cast<PresetItem*> (item);
	return 0;
}


void
PresetsListView::auto_open_selected()
{
	QTreeWidgetItem* item = selected_item();
	if (item)
		item->setExpanded (true);
}


void
PresetsListView::context_menu (QPoint const& pos)
{
	QTreeWidgetItem* item = itemAt (pos);
	if (!item)
		return;

	QMenu* menu = new QMenu (this);
	item->setSelected (true);

	if (item == 0)
	{
		menu->addAction (Resources::Icons16::presets_package(), "New pac&kage", _presets_manager, SLOT (create_package()));
	}
	else if (dynamic_cast<PackageItem*> (item))
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

	menu->exec (QCursor::pos());
	delete menu;
}


void
PresetsListView::dragEnterEvent (QDragEnterEvent* event)
{
	PresetsListView* source;
	if (event->source() && (source = dynamic_cast<PresetsListView*> (event->source())) &&
		source == this && event->mimeData()->hasText())
	{
		event->accept();
	}
	else
		event->ignore();
}


void
PresetsListView::dragMoveEvent (QDragMoveEvent* event)
{
	PresetsListView* source;
	if (event->source() && (source = dynamic_cast<PresetsListView*> (event->source())) &&
		source == this && event->mimeData()->hasText())
	{
		QTreeWidgetItem* to = drag_drop_item (event->pos());
		if (to && can_drop (_dragged_item, to))
			event->accept();
		else
			event->ignore();
	}
	else
		event->ignore();
}


void
PresetsListView::dragLeaveEvent (QDragLeaveEvent*)
{
	_dropped_on_item = 0;
}


void
PresetsListView::dropEvent (QDropEvent* event)
{
	PresetsListView* source;
	if (event->source() && (source = dynamic_cast<PresetsListView*> (event->source())) &&
		source == this && event->mimeData()->hasText())
	{
		QTreeWidgetItem* to = drag_drop_item (event->pos());
		if (_dragged_item && to)
		{
			PackageItem* package_item;
			CategoryItem* category_item;
			PresetItem* preset_item;

			// Move Preset to Category:
			if ((preset_item = dynamic_cast<PresetItem*> (_dragged_item)) && (category_item = dynamic_cast<CategoryItem*> (to)))
			{
				CategoryItem* old_category_item = preset_item->category_item();
				CategoryItem* new_category_item = category_item;
				old_category_item->takeChild (old_category_item->indexOfChild (preset_item));
				new_category_item->addChild (preset_item);
				new_category_item->setExpanded (true);
				preset_item->setSelected (true);
				// Save one or two files:
				old_category_item->package_item()->save_file();
				if (new_category_item->package_item() != old_category_item->package_item())
					new_category_item->package_item()->save_file();
			}
			// Move Category to Package:
			else if ((category_item = dynamic_cast<CategoryItem*> (_dragged_item)) && (package_item = dynamic_cast<PackageItem*> (to)))
			{
				PackageItem* old_package_item = category_item->package_item();
				PackageItem* new_package_item = package_item;
				old_package_item->takeChild (old_package_item->indexOfChild (category_item));
				new_package_item->addChild (category_item);
				new_package_item->setExpanded (true);
				category_item->setSelected (true);
				// Save one or two files:
				old_package_item->save_file();
				if (new_package_item != old_package_item)
					new_package_item->save_file();
			}
		}
	}
	event->acceptProposedAction();
	dragLeaveEvent (0);
}


void
PresetsListView::mousePressEvent (QMouseEvent* mouse_event)
{
	QTreeWidget::mousePressEvent (mouse_event);

	if (mouse_event->button() == Qt::LeftButton)
	{
		_drag_pos = mouse_event->pos();
		QTreeWidgetItem* item = itemAt (_drag_pos);
		if (item && item->flags() & Qt::ItemIsDragEnabled)
			_dragged_item = itemAt (_drag_pos);
	}
}


void
PresetsListView::mouseMoveEvent (QMouseEvent* mouse_event)
{
	QTreeWidget::mouseMoveEvent (mouse_event);

	if ((mouse_event->buttons() & Qt::LeftButton) && _dragged_item &&
		((mouse_event->pos() - _drag_pos).manhattanLength() >= QApplication::startDragDistance()))
	{
		QMimeData* mime_data = new QMimeData();
		mime_data->setText (_dragged_item->text (0));
		QDrag* drag = new QDrag (this);
		drag->setMimeData (mime_data);
		drag->setPixmap (_dragged_item->icon (0).pixmap (16));
		drag->setHotSpot (QPoint (-4, -12));
		drag->start (Qt::LinkAction);
		// We've dragged and maybe dropped it by now...
		_dragged_item = 0;
	}
}


bool
PresetsListView::can_drop (QTreeWidgetItem* from, QTreeWidgetItem* to)
{
	return (dynamic_cast<PresetItem*> (from) && dynamic_cast<CategoryItem*> (to) && from->parent() != to)
		|| (dynamic_cast<CategoryItem*> (from) && dynamic_cast<PackageItem*> (to) && from->parent() != to);
}


QTreeWidgetItem*
PresetsListView::drag_drop_item (QPoint const& epos)
{
	QTreeWidgetItem* item = itemAt (epos);
	if (item)
	{
		if (item != _dropped_on_item)
		{
			clearSelection();
			item->setSelected (true);
			_dropped_on_item = item;
			_auto_open_timer->start (750, true);
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

