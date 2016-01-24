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
 * --
 * Here be basic, global functions and macros like asserts, debugging helpers, etc.
 */

#ifndef HARUHI__CONFIG__RESOURCES_H__INCLUDED
#define HARUHI__CONFIG__RESOURCES_H__INCLUDED

// Qt:
#include <QtGui/QFont>
#include <QtGui/QPixmap>

// Haruhi:
#include <haruhi/config/all.h>


namespace Config {

extern int spacing();

extern int margin();

extern int window_margin();

extern int dialog_margin();

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

QPixmap
get_png_icon (QString const& png_file) noexcept;

QPixmap
get_svg_icon (QString const& svg_file) noexcept;

/*
 * Icons resources
 */

#define HARUHI_CONFIG_HAS_PNG_ICON(key, file)	\
	static inline QPixmap key()					\
	{											\
		return get_png_icon (file);				\
	}

#define HARUHI_CONFIG_HAS_SVG_ICON(key, file)	\
	static inline QPixmap key()					\
	{											\
		return get_svg_icon (file);				\
	}

	namespace Icons16 // TODO rename to Icons
	{
#define HARUHI_CONFIG_HAS_ICON(key, file) \
		HARUHI_CONFIG_HAS_SVG_ICON(key, HARUHI_SHARED_DIRECTORY "/images/svg/" file)

		HARUHI_CONFIG_HAS_ICON (haruhi, "haruhi.svg");
		HARUHI_CONFIG_HAS_ICON (all, "all.svg");
		HARUHI_CONFIG_HAS_ICON (add, "add.svg");
		HARUHI_CONFIG_HAS_ICON (rename, "rename.svg");
		HARUHI_CONFIG_HAS_ICON (remove, "remove.svg");
		HARUHI_CONFIG_HAS_ICON (menu, "menu.svg");
		HARUHI_CONFIG_HAS_ICON (configure, "configure.svg");
		HARUHI_CONFIG_HAS_ICON (colorpicker, "colorpicker.svg");
		HARUHI_CONFIG_HAS_ICON (template_, "template.svg");
		HARUHI_CONFIG_HAS_ICON (new_, "new.svg");
		HARUHI_CONFIG_HAS_ICON (ok, "ok.svg");
		HARUHI_CONFIG_HAS_ICON (keyboard, "keyboard.svg");
		HARUHI_CONFIG_HAS_ICON (session_manager, "session-manager.svg");
		HARUHI_CONFIG_HAS_ICON (insert, "insert.svg");
		HARUHI_CONFIG_HAS_ICON (load, "load.svg");
		HARUHI_CONFIG_HAS_ICON (save, "save.svg");
		HARUHI_CONFIG_HAS_ICON (save_as, "save_as.svg");
		HARUHI_CONFIG_HAS_ICON (open, "open.svg");
		HARUHI_CONFIG_HAS_ICON (exit, "exit.svg");
		HARUHI_CONFIG_HAS_ICON (connect, "connect.svg");
		HARUHI_CONFIG_HAS_ICON (disconnect, "disconnect.svg");
		HARUHI_CONFIG_HAS_ICON (unit, "unit.svg");
		HARUHI_CONFIG_HAS_ICON (port, "port.svg");
		HARUHI_CONFIG_HAS_ICON (port_group, "port-group.svg");
		HARUHI_CONFIG_HAS_ICON (audio_input_port, "audio-input-port.svg");
		HARUHI_CONFIG_HAS_ICON (audio_output_port, "audio-output-port.svg");
		HARUHI_CONFIG_HAS_ICON (event_input_port, "event-input-port.svg");
		HARUHI_CONFIG_HAS_ICON (event_output_port, "event-output-port.svg");
		HARUHI_CONFIG_HAS_ICON (event_polyphonic_input_port, "event-polyphonic-input-port.svg");
		HARUHI_CONFIG_HAS_ICON (event_polyphonic_output_port, "event-polyphonic-output-port.svg");
		HARUHI_CONFIG_HAS_ICON (input_unit, "input-unit.svg");
		HARUHI_CONFIG_HAS_ICON (output_unit, "output-unit.svg");
		HARUHI_CONFIG_HAS_ICON (input_unit_bay, "input-unit-bay.svg");
		HARUHI_CONFIG_HAS_ICON (output_unit_bay, "output-unit-bay.svg");
		HARUHI_CONFIG_HAS_ICON (presets_package, "presets-package.svg");
		HARUHI_CONFIG_HAS_ICON (presets_category, "presets-category.svg");
		HARUHI_CONFIG_HAS_ICON (preset, "preset.svg");
		HARUHI_CONFIG_HAS_ICON (preset_favorite, "preset-favorite.svg");
		HARUHI_CONFIG_HAS_ICON (wave, "wave.svg");
		HARUHI_CONFIG_HAS_ICON (wave_sine, "wave-sine.svg");
		HARUHI_CONFIG_HAS_ICON (wave_triangle, "wave-triangle.svg");
		HARUHI_CONFIG_HAS_ICON (wave_square, "wave-square.svg");
		HARUHI_CONFIG_HAS_ICON (wave_sawtooth, "wave-sawtooth.svg");
		HARUHI_CONFIG_HAS_ICON (wave_pulse, "wave-pulse.svg");
		HARUHI_CONFIG_HAS_ICON (wave_power, "wave-power.svg");
		HARUHI_CONFIG_HAS_ICON (wave_gauss, "wave-gauss.svg");
		HARUHI_CONFIG_HAS_ICON (wave_diode, "wave-diode.svg");
		HARUHI_CONFIG_HAS_ICON (wave_chirp, "wave-chirp.svg");
		HARUHI_CONFIG_HAS_ICON (wave_random_square, "wave-random-square.svg");
		HARUHI_CONFIG_HAS_ICON (wave_random_triangle, "wave-random-triangle.svg");
		HARUHI_CONFIG_HAS_ICON (wave_noise, "wave-noise.svg");
		HARUHI_CONFIG_HAS_ICON (modulator_ring, "modulator-ring.svg");
		HARUHI_CONFIG_HAS_ICON (modulator_fm, "modulator-fm.svg");
		HARUHI_CONFIG_HAS_ICON (adsr, "adsr.svg");
		HARUHI_CONFIG_HAS_ICON (eg, "eg.svg");
		HARUHI_CONFIG_HAS_ICON (lfo, "lfo.svg");
		HARUHI_CONFIG_HAS_ICON (mikuru, "mikuru.svg");
		HARUHI_CONFIG_HAS_ICON (favorite, "favorite.svg");
		HARUHI_CONFIG_HAS_ICON (modulator, "modulator.svg");
		HARUHI_CONFIG_HAS_ICON (filters, "filters.svg");
		HARUHI_CONFIG_HAS_ICON (effects, "effects.svg");
		HARUHI_CONFIG_HAS_ICON (filter_lpf, "filter-lpf.svg");
		HARUHI_CONFIG_HAS_ICON (filter_bpf, "filter-bpf.svg");
		HARUHI_CONFIG_HAS_ICON (filter_hpf, "filter-hpf.svg");
		HARUHI_CONFIG_HAS_ICON (filter_notch, "filter-notch.svg");
		HARUHI_CONFIG_HAS_ICON (filter_allpass, "filter-allpass.svg");
		HARUHI_CONFIG_HAS_ICON (filter_peaking, "filter-peaking.svg");
		HARUHI_CONFIG_HAS_ICON (filter_lowshelf, "filter-lowshelf.svg");
		HARUHI_CONFIG_HAS_ICON (filter_highshelf, "filter-highshelf.svg");
		HARUHI_CONFIG_HAS_ICON (main, "main.svg");
		HARUHI_CONFIG_HAS_ICON (parts, "parts.svg");
		HARUHI_CONFIG_HAS_ICON (dialog_ok, "dialog-ok.svg");
		HARUHI_CONFIG_HAS_ICON (dialog_ok_apply, "dialog-ok-apply.svg");
		HARUHI_CONFIG_HAS_ICON (dialog_cancel, "dialog-cancel.svg");
		HARUHI_CONFIG_HAS_ICON (clear_list, "clear-list.svg");
		HARUHI_CONFIG_HAS_ICON (panic, "panic.svg");
		HARUHI_CONFIG_HAS_ICON (unit_type_effect, "unit-type-effect.svg");
		HARUHI_CONFIG_HAS_ICON (unit_type_eg, "unit-type-eg.svg");
		HARUHI_CONFIG_HAS_ICON (unit_type_arp, "unit-type-arp.svg");
		HARUHI_CONFIG_HAS_ICON (unit_type_synth, "unit-type-synth.svg");
		HARUHI_CONFIG_HAS_ICON (connections, "connections.svg");
		HARUHI_CONFIG_HAS_ICON (audio, "audio.svg");
		HARUHI_CONFIG_HAS_ICON (event, "event.svg");

#undef HARUHI_CONFIG_HAS_ICON
	} // namespace Icons16

	namespace NoteIcons
	{
#define HARUHI_CONFIG_HAS_NOTE_ICON(key, file) \
		HARUHI_CONFIG_HAS_PNG_ICON(key, HARUHI_SHARED_DIRECTORY "/images/notes/" file)

		HARUHI_CONFIG_HAS_NOTE_ICON (note_01, "note-01.png");
		HARUHI_CONFIG_HAS_NOTE_ICON (note_02, "note-02.png");
		HARUHI_CONFIG_HAS_NOTE_ICON (note_04, "note-04.png");
		HARUHI_CONFIG_HAS_NOTE_ICON (note_08, "note-08.png");
		HARUHI_CONFIG_HAS_NOTE_ICON (note_16, "note-16.png");
		HARUHI_CONFIG_HAS_NOTE_ICON (note_32, "note-32.png");

#undef HARUHI_CONFIG_HAS_NOTE_ICON
	} // namespace NoteIcons

#undef HARUHI_CONFIG_HAS_SVG_ICON
#undef HARUHI_CONFIG_HAS_PNG_ICON

} // namespace Config

#endif

