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

#ifndef HARUHI__PLUGINS__YUKI__PART_MODULATOR_H__INCLUDED
#define HARUHI__PLUGINS__YUKI__PART_MODULATOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <list>
#include <vector>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QCheckBox>

// Haruhi:
#include <haruhi/config/all.h>

// Local:
#include "params.h"
#include "operator_widget.h"


namespace Yuki {

using Haruhi::Unique;

class Part;
class PartWidget;


class PartModulatorWidget: public QWidget
{
	Q_OBJECT

	// Modulation matrix knobs [0..3][0..2]:
	typedef std::vector<std::vector<Unique<Haruhi::Knob>>>	MatrixKnobs;

  public:
	PartModulatorWidget (QWidget* parent, PartWidget* part_widget, Part* part);

  public slots:
	/**
	 * Called by PartWidget.
	 */
	void
	widgets_to_wave_params();

	/**
	 * Called by PartWidget.
	 */
	void
	widgets_to_oscillator_params();

	/**
	 * Called by PartWidget.
	 */
	void
	update_widgets();

	/**
	 * Update widgets deps (enable/disable, etc.)
	 */
	void
	params_to_widgets();

  private:
	/**
	 * Create new label and add it to _modulator_labels.
	 */
	QLabel*
	create_modulator_label (QString const& text);

  private:
	PartWidget*			_part_widget;
	Part*				_part;

	// Modulation matrix knobs:
	MatrixKnobs			_fm_matrix_knobs;
	MatrixKnobs			_am_matrix_knobs;

	OperatorWidget*		_operator_1;
	OperatorWidget*		_operator_2;
	OperatorWidget*		_operator_3;
	QCheckBox*			_modulator_enabled;
	std::list<QWidget*>	_modulator_labels;
};

} // namespace Yuki

#endif

