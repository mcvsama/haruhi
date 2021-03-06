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

#ifndef HARUHI__COMPONENTS__AUDIO_BACKEND__TREE_H__INCLUDED
#define HARUHI__COMPONENTS__AUDIO_BACKEND__TREE_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Haruhi:
#include <haruhi/config/all.h>
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
	backend() const;

	QTreeWidgetItem*
	selected_item() const;

	void
	save_state (QDomElement&) const override;

	void
	load_state (QDomElement const&) override;

  private:
	Backend* _backend;
};


inline Backend*
Tree::backend() const
{
	return _backend;
}

} // namespace AudioBackendImpl

} // namespace Haruhi

#endif

