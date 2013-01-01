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

#ifndef HARUHI__SESSION__PROGRAM_H__INCLUDED
#define HARUHI__SESSION__PROGRAM_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/saveable_state.h>
#include <haruhi/session/patch.h>


namespace Haruhi {

class Session;


class Program:
	public QWidget,
	public SaveableState
{
  public:
	Program (Session*, QWidget* parent = 0);

	virtual ~Program();

	void
	save_state (QDomElement&) const override;

	void
	load_state (QDomElement const&) override;

  private:
	Patch* _patch;
};

} // namespace Haruhi

#endif

