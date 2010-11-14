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
#include <QtGui/QDragEnterEvent>
#include <Qt3Support/Q3PopupMenu>
#include <Qt3Support/Q3Header>
#include <Qt3Support/Q3DragObject>
#include <Qt3Support/Q3ListView>
#include <Qt3Support/Q3TextDrag>

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
	Q3ListView (parent),
	_presets_manager (presets_manager),
	_dragged_item (0),
	_dropped_on_item (0)
{
	header()->setClickEnabled (false);
	header()->setResizeEnabled (false);
	header()->hide();
	setMinimumWidth (160);
	setSorting (0);
	setSelectionMode (Q3ListView::Single);
	setRootIsDecorated (true);
	setAllColumnsShowFocus (true);
	setColumnWidthMode (0, Q3ListView::Maximum);
	setResizeMode (Q3ListView::AllColumns);
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	setMultiSelection (false);
	addColumn ("Presets");
	setAcceptDrops (true);

	_auto_open_timer = new QTimer (this);
	QObject::connect (_auto_open_timer, SIGNAL (timeout()), this, SLOT (auto_open_selected()));
	QObject::connect (this, SIGNAL (contextMenuRequested (Q3ListViewItem*, const QPoint&, int)), this, SLOT (context_menu (Q3ListViewItem*, const QPoint&, int)));
}


PresetsListView::~PresetsListView()
{
	_auto_open_timer->stop();
}


PackageItem*
PresetsListView::current_package_item() const
{
	Q3ListViewItem* item = selectedItem();
	if (item)
		return dynamic_cast<PackageItem*> (item);
	return 0;
}


CategoryItem*
PresetsListView::current_category_item() const
{
	Q3ListViewItem* item = selectedItem();
	if (item)
		return dynamic_cast<CategoryItem*> (item);
	return 0;
}


PresetItem*
PresetsListView::current_preset_item() const
{
	Q3ListViewItem* item = selectedItem();
	if (item)
		return dynamic_cast<PresetItem*> (item);
	return 0;
}


void
PresetsListView::auto_open_selected()
{
	Q3ListViewItem* item = selectedItem();
	if (item)
		setOpen (item, true);
}


void
PresetsListView::context_menu (Q3ListViewItem* item, QPoint const& pos, int col)
{
	Q3PopupMenu* menu = new Q3PopupMenu (this);
	setSelected (item, true);

	if (item == 0)
	{
		menu->insertItem (Config::Icons16::presets_package(), "New pac&kage", _presets_manager, SLOT (create_package()));
	}
	else if (dynamic_cast<PackageItem*> (item))
	{
		menu->insertItem (Config::Icons16::presets_category(), "New &category", _presets_manager, SLOT (create_category()));
		menu->insertItem (Config::Icons16::presets_package(), "New pac&kage", _presets_manager, SLOT (create_package()));
		menu->insertSeparator();
		menu->insertItem (Config::Icons16::remove(), "Destroy package", _presets_manager, SLOT (destroy()));
	}
	else if (dynamic_cast<CategoryItem*> (item))
	{
		menu->insertItem (Config::Icons16::preset(), "New &preset", _presets_manager, SLOT (create_preset()));
		menu->insertItem (Config::Icons16::presets_category(), "New &category", _presets_manager, SLOT (create_category()));
		menu->insertItem (Config::Icons16::presets_package(), "New pac&kage", _presets_manager, SLOT (create_package()));
		menu->insertSeparator();
		menu->insertItem (Config::Icons16::remove(), "Destroy category", _presets_manager, SLOT (destroy()));
	}
	else if (dynamic_cast<PresetItem*> (item))
	{
		menu->insertItem (Config::Icons16::load(), "&Load preset", _presets_manager, SLOT (load_preset()));
		menu->insertSeparator();
		menu->insertItem (Config::Icons16::preset(), "New &preset", _presets_manager, SLOT (create_preset()));
		menu->insertItem (Config::Icons16::presets_category(), "New &category", _presets_manager, SLOT (create_category()));
		menu->insertItem (Config::Icons16::presets_package(), "New pac&kage", _presets_manager, SLOT (create_package()));
		menu->insertSeparator();
		menu->insertItem (Config::Icons16::remove(), "Destroy preset", _presets_manager, SLOT (destroy()));
	}

	menu->exec (pos);
	delete menu;
}


void
PresetsListView::dragEnterEvent (QDragEnterEvent* event)
{
	PresetsListView* source;
	if (event->source() && (source = dynamic_cast<PresetsListView*> (event->source())) && source == this && Q3TextDrag::canDecode (event))
	{
		Q3ListViewItem* to = drag_drop_item (event->pos());
		if (to && can_drop (_dragged_item, to))
			event->accept (itemRect (to));
		else
			event->ignore();
	}
	else
		event->ignore();
}


void
PresetsListView::dragMoveEvent (QDragMoveEvent* event)
{
	PresetsListView* source;
	if (event->source() && (source = dynamic_cast<PresetsListView*> (event->source())) && source == this && Q3TextDrag::canDecode (event))
	{
		Q3ListViewItem* to = drag_drop_item (event->pos());
		if (to && can_drop (_dragged_item, to))
			event->accept (itemRect (to));
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
	_dragged_item = 0;
}


void
PresetsListView::dropEvent (QDropEvent* event)
{
	PresetsListView* source;
	if (event->source() && (source = dynamic_cast<PresetsListView*> (event->source())) && source == this && Q3TextDrag::canDecode (event))
	{
		Q3ListViewItem* to = drag_drop_item (event->pos());
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
				old_category_item->takeItem (preset_item);
				new_category_item->insertItem (preset_item);
				new_category_item->setOpen (true);
				setSelected (preset_item, true);
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
				old_package_item->takeItem (category_item);
				new_package_item->insertItem (category_item);
				new_package_item->setOpen (true);
				setSelected (category_item, true);
				// Save one or two files:
				old_package_item->save_file();
				if (new_package_item != old_package_item)
					new_package_item->save_file();
			}
		}
	}
	dragLeaveEvent (0);
}


bool
PresetsListView::can_drop (Q3ListViewItem* from, Q3ListViewItem* to)
{
	return (dynamic_cast<PresetItem*> (from) && dynamic_cast<CategoryItem*> (to) && from->parent() != to)
		|| (dynamic_cast<CategoryItem*> (from) && dynamic_cast<PackageItem*> (to) && from->parent() != to);
}


Q3DragObject*
PresetsListView::dragObject()
{
	Q3TextDrag* drag = 0;
	_dragged_item = currentItem();
	if (_dragged_item && _dragged_item->dragEnabled())
	{
		drag = new Q3TextDrag (_dragged_item->text (0), this);
		QPixmap const* pixmap = _dragged_item->pixmap (0);
		if (pixmap)
			drag->setPixmap (*pixmap, QPoint (-4, -12));
	}
	return drag;
}


Q3ListViewItem*
PresetsListView::drag_drop_item (QPoint const& epos)
{
	Q3ListViewItem *item = itemAt (epos);
	int m = 0;
	if (item)
	{
		m = item->height();
		if (item != _dropped_on_item)
		{
			setSelected (item, true);
			_dropped_on_item = item;
			_auto_open_timer->start (750, true);
			if (!item->dropEnabled())
				item = 0;
		}
	}
	else
	{
		_dropped_on_item = 0;
		_auto_open_timer->stop();
	}
	QPoint vpos = viewportToContents (epos);
	ensureVisible (vpos.x(), vpos.y(), m, m);
	return item;
}

} // namespace PresetsManagerPrivate

} // namespace Haruhi

