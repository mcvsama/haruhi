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
#include <haruhi/lib/controller.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/widgets/dial_control.h>


namespace Haruhi {

class Knob;


/**
 * Can't insert it into Knob namespace due to Qt's MOC restrictions.
 */
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

		bool
		volume_scale() const { return _volume_scale; }

		void
		set_volume_scale (bool setting, float exp = 1.0f);

		/**
		 * Detached knob works like a simple QDoubleSpinBox,
		 * that is it does not care about any knob.
		 */
		void
		set_detached (bool setting) { _detached = setting; }

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
		bool				_volume_scale;
		float				_volume_scale_exp;
	};

	typedef std::map<int, Port*> ContextMenuPortMap;

  public:
	/**
	 * Creates Knob.
	 *
	 * \param	parent: Parent widget.
	 * \param	label: Displayed label.
	 * \param	show_min, show_max: Values range shown in spinbox.
	 * \param	step: Change step.
	 * \param	decimals: How many decimal digits should be shown in spinbox.
	 */
	Knob (QWidget* parent, EventPort* event_port, ControllerParam* controller_param,
		  QString const& label, float show_min, float show_max, int step, int decimals);

	/**
	 * Enables/disables volume scale (shown in dB).
	 * \param	exp is power value, usually M_E.
	 */
	void
	set_volume_scale (bool setting, float exp = 1.0f) { _spin_box->set_volume_scale (setting, exp); }

	/**
	 * Reads ControllerProxy::Config and updates widgets.
	 * \entry	UI thread only.
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
	 * \entry	UI thread only.
	 */
	void
	reset();

	/**
	 * Call configuration dialog.
	 * \entry	UI thread only.
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
	create_connect_menu (QMenu*, Unit*, QPixmap const& pixmap_for_port_group, QPixmap const& pixmap_for_port);

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

