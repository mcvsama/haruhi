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

#ifndef HARUHI__WIDGETS__KNOB_H__INCLUDED
#define HARUHI__WIDGETS__KNOB_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QSignalMapper>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QMenu>
#include <QtGui/QDialog>

// Haruhi:
#include <haruhi/controller_proxy.h>
#include <haruhi/widgets/controller.h>
#include <haruhi/widgets/dial_control.h>


namespace Haruhi {

class Knob;


class KnobProperties: public QDialog
{
	Q_OBJECT

  public:
	KnobProperties (Knob* knob, QWidget* parent);

	void
	apply();

  private slots:
	void
	limit_min_updated();

	void
	limit_max_updated();

  private:
	Knob*		_knob;
	QSpinBox*	_curve_spinbox;
	QSpinBox*	_user_limit_min_spinbox;
	QSpinBox*	_user_limit_max_spinbox;
	QSpinBox*	_smoothing_spinbox;
};


/**
 * Knob for parameter manipulation.
 * Knob should be always deleted before ControllerProxies, to prevent
 * race conditions from PeriodicUpdater.
 */
class Knob:
	public QFrame,
	public Controller
{
	Q_OBJECT

	friend class KnobProperties;

	/**
	 * SpinBox used by Knob, below DialControl.
	 */
	class SpinBox: public QSpinBox
	{
	  public:
		SpinBox (QWidget* parent, Knob* knob, int user_limit_min, int user_limit_max, float show_min, float show_max, int step, int decimals);

		/*
		 * QSpinBox API
		 */

		QValidator::State
		validate (QString&, int&) const;

		float
		show_min() const { return _show_min; }

		float
		show_max() const { return _show_max; }

		int
		decimals() const { return _decimals; }

		/**
		 * Detached knob works like a simple QDoubleSpinBox.
		 * Set as deatched when spin box is not main Knob's spin box.
		 */
		void
		set_detached (bool setting);

	  protected:
		/*
		 * QSpinBox API
		 */

		QString
		textFromValue (int value) const;

		int
		valueFromText (QString const& text) const;

	  private:
		float
		int_to_float (int) const;

		int
		float_to_int (float) const;

	  private:
		Knob*				_knob;
		float				_show_min;
		float				_show_max;
		int					_decimals;
		QDoubleValidator*	_validator;
		bool				_detached;
	};

	typedef std::map<int, Core::Port*> ContextMenuPortMap;

  public:
	/**
	 * Creates Knob.
	 *
	 * \param	parent: Parent widget.
	 * \param	controller_proxy: ControllerProxy object, must be present.
	 * \param	label: Displayed label.
	 * \param	show_min, show_max: Values range shown in spinbox.
	 * \param	step: Change step.
	 * \param	decimals: How many decimal digits should be shown in spinbox.
	 */
	Knob (QWidget* parent, ControllerProxy* controller_proxy, QString const& label, float show_min, float show_max, int step, int decimals);

	~Knob();

	/**
	 * Reads ControllerProxy::Config and updates widgets.
	 * \entry	Qt thread only.
	 */
	void
	read_config();

	/**
	 * Forces Knob to read parameter value and update itself.
	 * Also sends changed() signal.
	 */
	void
	read();

	/*
	 * PeriodicUpdater::Receiver API
	 */

	/**
	 * Calls read().
	 */
	void
	periodic_update();

  public slots:
	/**
	 * Reset Knob to default value.
	 * \entry	Qt thread only.
	 */
	void
	reset();

	/**
	 * Call configuration dialog.
	 * \entry	Qt thread only.
	 */
	void
	configure();

  private:
	void
	update_widgets();

	/**
	 * Creates menu for right-click.
	 */
	QMenu*
	create_context_menu();

	/**
	 * Populates given menu with options for connecting knob to event ports.
	 */
	void
	create_connect_menu (QMenu*, Core::Unit*, QPixmap const& pixmap_for_port_group, QPixmap const& pixmap_for_port);

	// Controller API:
	void
	learning_state_changed();

  protected:
	void
	mousePressEvent (QMouseEvent*);

	void
	mouseDoubleClickEvent (QMouseEvent*);

  private slots:
	void
	dial_changed (int);

	void
	spin_changed (int);

	void
	connect_port (int action_id);

	void
	disconnect_port (int action_id);

	void
	disconnect_from_all();

	void
	start_learning_slot() { start_learning(); }

	void
	stop_learning_slot() { stop_learning(); }

  signals:
	void
	changed (int);

  private:
	bool				_prevent_recursion;
	QSignalMapper*		_connect_signal_mapper;
	QSignalMapper*		_disconnect_signal_mapper;
	ContextMenuPortMap	_context_menu_port_map;
	int					_action_id; // Helper for generating new IDs for _signal_mapper.
	QColor				_std_text_color;

	// Widgets:
	QLabel*				_label;
	DialControl*		_dial_control;
	SpinBox*			_spin_box;
	QMenu*				_context_menu;
	QMenu*				_connect_menu;
	QMenu*				_disconnect_menu;
};

} // namespace Haruhi

#endif

