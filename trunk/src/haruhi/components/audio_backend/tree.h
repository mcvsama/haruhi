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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__PORTS_LIST_VIEW_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__PORTS_LIST_VIEW_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/utility/saveable_state.h>


namespace Haruhi {

namespace AudioBackendImpl {

class Backend;

class Tree:
	public QTreeWidget,
	public SaveableState
{
  public:
	Tree (QWidget* parent, Backend*, const char* header_title);

	Backend*
	backend() const { return _backend; }

	QTreeWidgetItem*
	selected_item() const;

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  private:
	Backend* _backend;
};

} // namespace AudioBackendImpl

} // namespace Haruhi

#endif

