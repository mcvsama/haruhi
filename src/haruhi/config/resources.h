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
 * --
 * Here be basic, global functions and macros like asserts, debugging helpers, etc.
 */

#ifndef HARUHI__CONFIG__RESOURCES_H__INCLUDED
#define HARUHI__CONFIG__RESOURCES_H__INCLUDED

// Qt:
#include <QtGui/QFont>
#include <QtGui/QPixmap>
#include <QtGui/QPixmapCache>

// Haruhi:
#include <haruhi/config/all.h>


namespace Config {

enum {
	Spacing			= 3,
	Margin			= 2,
	SmallSpacing	= 2,
	WindowMargin	= 4,
	DialogMargin	= 6
};

} // namespace Config


#ifndef HARUHI_PREFIX
#define HARUHI_PREFIX ""
#endif

#ifndef HARUHI_SHARED_DIRECTORY
#define HARUHI_SHARED_DIRECTORY HARUHI_PREFIX "share"
#endif

#ifndef HARUHI_XDG_SETTINGS_HOME
#define HARUHI_XDG_SETTINGS_HOME "mulabs.org/haruhi"
#endif

#ifndef HARUHI_XDG_DATA_HOME
#define HARUHI_XDG_DATA_HOME "mulabs.org/haruhi"
#endif


namespace Resources {

QFont&
small_font();

/*
 * Icons resources
 */

#define HARUHI_CONFIG_HAS_ICON(key, file)				\
	static inline QPixmap key()							\
	{													\
		QPixmap p;										\
		if (!QPixmapCache::find (#key, p))				\
		{												\
			p = QPixmap (file);							\
			QPixmapCache::insert (#key, p);				\
		}												\
		return p;										\
	}

	namespace Icons16
	{
#define HARUHI_CONFIG_HAS_ICON_16(key, file) \
		HARUHI_CONFIG_HAS_ICON(key, HARUHI_SHARED_DIRECTORY"/images/16/"#file)

		HARUHI_CONFIG_HAS_ICON_16 (haruhi, "haruhi.png");
		HARUHI_CONFIG_HAS_ICON_16 (all, "all.png");
		HARUHI_CONFIG_HAS_ICON_16 (add, "add.png");
		HARUHI_CONFIG_HAS_ICON_16 (rename, "rename.png");
		HARUHI_CONFIG_HAS_ICON_16 (remove, "remove.png");
		HARUHI_CONFIG_HAS_ICON_16 (configure, "configure.png");
		HARUHI_CONFIG_HAS_ICON_16 (colorpicker, "colorpicker.png");
		HARUHI_CONFIG_HAS_ICON_16 (template_, "template.png");
		HARUHI_CONFIG_HAS_ICON_16 (new_, "new.png");
		HARUHI_CONFIG_HAS_ICON_16 (ok, "ok.png");
		HARUHI_CONFIG_HAS_ICON_16 (keyboard, "keyboard.png");
		HARUHI_CONFIG_HAS_ICON_16 (session_manager, "session-manager.png");
		HARUHI_CONFIG_HAS_ICON_16 (insert, "insert.png");
		HARUHI_CONFIG_HAS_ICON_16 (load, "load.png");
		HARUHI_CONFIG_HAS_ICON_16 (save, "save.png");
		HARUHI_CONFIG_HAS_ICON_16 (save_as, "save_as.png");
		HARUHI_CONFIG_HAS_ICON_16 (open, "open.png");
		HARUHI_CONFIG_HAS_ICON_16 (exit, "exit.png");
		HARUHI_CONFIG_HAS_ICON_16 (connect, "connect.png");
		HARUHI_CONFIG_HAS_ICON_16 (disconnect, "disconnect.png");
		HARUHI_CONFIG_HAS_ICON_16 (unit, "unit.png");
		HARUHI_CONFIG_HAS_ICON_16 (port, "port.png");
		HARUHI_CONFIG_HAS_ICON_16 (port_group, "port-group.png");
		HARUHI_CONFIG_HAS_ICON_16 (audio_input_port, "audio-input-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (audio_output_port, "audio-output-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (event_input_port, "event-input-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (event_output_port, "event-output-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (event_polyphonic_input_port, "event-polyphonic-input-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (event_polyphonic_output_port, "event-polyphonic-output-port.png");
		HARUHI_CONFIG_HAS_ICON_16 (input_unit, "input-unit.png");
		HARUHI_CONFIG_HAS_ICON_16 (output_unit, "output-unit.png");
		HARUHI_CONFIG_HAS_ICON_16 (input_unit_bay, "input-unit-bay.png");
		HARUHI_CONFIG_HAS_ICON_16 (output_unit_bay, "output-unit-bay.png");
		HARUHI_CONFIG_HAS_ICON_16 (show_program, "show-program.png");
		HARUHI_CONFIG_HAS_ICON_16 (show_audio, "show-audio.png");
		HARUHI_CONFIG_HAS_ICON_16 (show_event, "show-event.png");
		HARUHI_CONFIG_HAS_ICON_16 (show_backends, "show-backends.png");
		HARUHI_CONFIG_HAS_ICON_16 (presets_package, "presets-package.png");
		HARUHI_CONFIG_HAS_ICON_16 (presets_category, "presets-category.png");
		HARUHI_CONFIG_HAS_ICON_16 (preset, "preset.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_sine, "wave-sine.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_triangle, "wave-triangle.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_square, "wave-square.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_sawtooth, "wave-sawtooth.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_pulse, "wave-pulse.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_power, "wave-power.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_gauss, "wave-gauss.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_diode, "wave-diode.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_chirp, "wave-chirp.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_random_square, "wave-random-square.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_random_triangle, "wave-random-triangle.png");
		HARUHI_CONFIG_HAS_ICON_16 (wave_noise, "wave-noise.png");
		HARUHI_CONFIG_HAS_ICON_16 (modulator_ring, "modulator-ring.png");
		HARUHI_CONFIG_HAS_ICON_16 (modulator_fm, "modulator-fm.png");
		HARUHI_CONFIG_HAS_ICON_16 (adsr, "adsr.png");
		HARUHI_CONFIG_HAS_ICON_16 (eg, "eg.png");
		HARUHI_CONFIG_HAS_ICON_16 (lfo, "lfo.png");
		HARUHI_CONFIG_HAS_ICON_16 (mikuru, "mikuru.png");
		HARUHI_CONFIG_HAS_ICON_16 (favorite, "favorite.png");
		HARUHI_CONFIG_HAS_ICON_16 (modulator, "modulator.png");
		HARUHI_CONFIG_HAS_ICON_16 (filters, "filters.png");
		HARUHI_CONFIG_HAS_ICON_16 (effects, "effects.png");
		HARUHI_CONFIG_HAS_ICON_16 (filter_lpf, "filter-lpf.png");
		HARUHI_CONFIG_HAS_ICON_16 (filter_bpf, "filter-bpf.png");
		HARUHI_CONFIG_HAS_ICON_16 (filter_hpf, "filter-hpf.png");
		HARUHI_CONFIG_HAS_ICON_16 (filter_notch, "filter-notch.png");
		HARUHI_CONFIG_HAS_ICON_16 (filter_allpass, "filter-allpass.png");
		HARUHI_CONFIG_HAS_ICON_16 (filter_peaking, "filter-peaking.png");
		HARUHI_CONFIG_HAS_ICON_16 (filter_lowshelf, "filter-lowshelf.png");
		HARUHI_CONFIG_HAS_ICON_16 (filter_highshelf, "filter-highshelf.png");
		HARUHI_CONFIG_HAS_ICON_16 (main, "main.png");
		HARUHI_CONFIG_HAS_ICON_16 (parts, "parts.png");
		HARUHI_CONFIG_HAS_ICON_16 (dialog_ok, "dialog-ok.png");
		HARUHI_CONFIG_HAS_ICON_16 (dialog_ok_apply, "dialog-ok-apply.png");
		HARUHI_CONFIG_HAS_ICON_16 (dialog_cancel, "dialog-cancel.png");
		HARUHI_CONFIG_HAS_ICON_16 (clear_list, "clear-list.png");

#undef HARUHI_CONFIG_HAS_ICON_16
	} // namespace Icons16

	namespace Icons22
	{
#define HARUHI_CONFIG_HAS_ICON_22(key, file) \
		HARUHI_CONFIG_HAS_ICON(key, HARUHI_SHARED_DIRECTORY"/images/22/"#file)

		HARUHI_CONFIG_HAS_ICON_22 (spacer, "spacer.png");
		HARUHI_CONFIG_HAS_ICON_22 (show_audio, "show-audio.png");
		HARUHI_CONFIG_HAS_ICON_22 (show_event, "show-event.png");
		HARUHI_CONFIG_HAS_ICON_22 (connections, "connections.png");
		HARUHI_CONFIG_HAS_ICON_22 (configure, "configure.png");

#undef HARUHI_CONFIG_HAS_ICON_22
	} // namespace Icons22

	namespace NoteIcons
	{
#define HARUHI_CONFIG_HAS_NOTE_ICON(key, file) \
		HARUHI_CONFIG_HAS_ICON(key, HARUHI_SHARED_DIRECTORY"/images/notes/"#file)

		HARUHI_CONFIG_HAS_NOTE_ICON (note_01, "note-01.png");
		HARUHI_CONFIG_HAS_NOTE_ICON (note_02, "note-02.png");
		HARUHI_CONFIG_HAS_NOTE_ICON (note_04, "note-04.png");
		HARUHI_CONFIG_HAS_NOTE_ICON (note_08, "note-08.png");
		HARUHI_CONFIG_HAS_NOTE_ICON (note_16, "note-16.png");
		HARUHI_CONFIG_HAS_NOTE_ICON (note_32, "note-32.png");

#undef HARUHI_CONFIG_HAS_NOTE_ICON
	} // namespace NoteIcons

#undef HARUHI_CONFIG_HAS_ICON

} // namespace Config

#endif

