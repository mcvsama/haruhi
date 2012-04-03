/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef HARUHI__PLUGINS__EG__KEY_MANAGER_H__INCLUDED
#define HARUHI__PLUGINS__EG__KEY_MANAGER_H__INCLUDED

// Standard:
#include <cstddef>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/utility/saveable_state.h>


namespace EG {

class KeyManager:
	public SaveableState
{
	enum class KeyMode
	{
		Mono,		// Max. one oscillator, retrigger notes.
		Legato,		// Max. one oscillator, no retriggering - just updates frequency.
		Poly,		// Polyphonic (multiple oscillators)
	};

	enum class SustainMode
	{
		Constant,	// Constant sustain volume.
		Fading,		// Fading out sustani volume.
	};

  public:
	// Ctor
	KeyManager();

	/**
	 * Get current key mode.
	 */
	KeyMode
	key_mode() const noexcept;

	/**
	 * Set key mode.
	 */
	void
	set_key_mode (KeyMode key_mode) noexcept;

	/**
	 * Get current sustain mode.
	 */
	SustainMode
	sustain_mode() const noexcept;

	/**
	 * Set sustain mode.
	 */
	void
	set_sustain_mode (SustainMode sustain_mode) noexcept;

	/**
	 * Return true if recatching (resustaining) is enabled.
	 */
	bool
	recatching() const noexcept;

	/**
	 * Enable/disable recatching.
	 */
	void
	set_recatching (bool enabled) noexcept;

	/**
	 * Return true if pitch-bending released keys is enabled.
	 */
	bool
	bend_released() const noexcept;

	/**
	 * Enable/disable pitch-bending released keys.
	 */
	void
	set_bend_released (bool enabled) noexcept;

	/*
	 * SaveableState implementation.
	 */

	void
	save_state (QDomElement&) const;

	void
	load_state (QDomElement const&);

  private:
	KeyMode		_key_mode;
	SustainMode	_sustain_mode;
	bool		_recatching;
	bool		_bend_released;
};


inline KeyManager::KeyMode
KeyManager::key_mode() const noexcept
{
	return _key_mode;
}


inline void
KeyManager::set_key_mode (KeyMode key_mode) noexcept
{
	_key_mode = key_mode;
}


inline KeyManager::SustainMode
KeyManager::sustain_mode() const noexcept
{
	return _sustain_mode;
}


inline void
KeyManager::set_sustain_mode (SustainMode sustain_mode) noexcept
{
	_sustain_mode = sustain_mode;
}


inline bool
KeyManager::recatching() const noexcept
{
	return _recatching;
}


inline void
KeyManager::set_recatching (bool enabled) noexcept
{
	_recatching = enabled;
}


inline bool
KeyManager::bend_released() const noexcept
{
	return _bend_released;
}


inline void
KeyManager::set_bend_released (bool enabled) noexcept
{
	_bend_released = enabled;
}

} // namespace EG

#endif

