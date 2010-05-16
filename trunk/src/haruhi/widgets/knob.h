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

#ifndef HARUHI__WIDGET__KNOB_H__INCLUDED
#define HARUHI__WIDGET__KNOB_H__INCLUDED

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
#include <haruhi/unit_bay.h>
#include <haruhi/controller_proxy.h>
#include <haruhi/widgets/dial_control.h>
#include <haruhi/components/event_backend/event_backend.h>


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
 */
class Knob:
	public QFrame,
	public ControllerProxy::Widget,
	public Haruhi::EventBackend::Learnable
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
	};

	typedef std::map<int, Core::Port*> ContextMenuPortMap;

  public:
	/**
	 * Creates Knob.
	 *
	 * \param	parent: Parent widget.
	 * \param	controller_proxy: ControllerProxy object, must not be 0.
	 * \param	label: Displayed label.
	 * \param	show_min, show_max: Values range shown in spinbox.
	 * \param	step: Change step.
	 * \param	decimals: How many decimal digits should be shown in spinbox.
	 */
	Knob (QWidget* parent, ControllerProxy* controller_proxy, QString const& label, float show_min, float show_max, int step, int decimals);

	~Knob();

	ControllerProxy*
	controller_proxy() const { return _controller_proxy; }

	/**
	 * Sets unit bay so Knob can display in popup menu list of ports it can connect to.
	 */
	void
	set_unit_bay (Haruhi::UnitBay* unit_bay) { _unit_bay = unit_bay; }

	/**
	 * Reads ControllerProxy::Config and updates widgets.
	 * \entry	Qt thread only.
	 */
	void
	read_config();

	/*
	 * PeriodicUpdater::Receiver API
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

	/**
	 * \entry	Qt thread only.
	 */
	void
	start_learning();

	/**
	 * \entry	Qt thread only.
	 */
	void
	stop_learning();

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

	// EventBackend::Learnable API:
	void
	learned_port (Haruhi::EventBackend::EventTypes, Core::EventPort*);

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

  signals:
	void
	changed (int);

  private:
	ControllerProxy*	_controller_proxy;
	Haruhi::UnitBay*	_unit_bay;
	bool				_prevent_recursion;
	QSignalMapper*		_connect_signal_mapper;
	QSignalMapper*		_disconnect_signal_mapper;
	ContextMenuPortMap	_context_menu_port_map;
	int					_action_id; // Helper for generating new IDs for _signal_mapper.
	bool				_learning;
	QColor				_std_text_color;

	// Widgets:
	QLabel*				_label;
	DialControl*		_dial_control;
	SpinBox*			_spin_box;
	QMenu*				_context_menu;
	QMenu*				_connect_menu;
	QMenu*				_disconnect_menu;
};

#endif

