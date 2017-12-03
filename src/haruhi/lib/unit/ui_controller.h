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

#ifndef HARUHI__LIB__UNIT__UI_CONTROLLER_H__INCLUDED
#define HARUHI__LIB__UNIT__UI_CONTROLLER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>


namespace Haruhi {

/**
 * Base class for UI controller widgets.
 */
class UIController
{
  public:
	/**
	 * API for UIController for UI thread.
	 */
	class UIThreadAPI
	{
	  public:
		void
		start_learning();

		void
		stop_learning();
	};

	// TODO move learning stuff to EventBackend class.

  protected:
	/**
	 * Called whenever learning is started/stopped. To be reimplemented in subclass, as the widget may want
	 * indicate the status to the user.
	 *
	 * \entry
	 *		UI thread only
	 */
	virtual void
	learning_state_changed()
	{ }

	/**
	 * Returns true if Controller is in 'learning' mode.
	 */
	bool
	learning() const noexcept;
};

} // namespace Haruhi

#endif

