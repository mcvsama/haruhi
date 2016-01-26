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

#ifndef HARUHI__WIDGETS__KNOB_H__INCLUDED
#define HARUHI__WIDGETS__KNOB_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QSignalMapper>
#include <QFrame>
#include <QLabel>
#include <QSpinBox>
#include <QMenu>
#include <QDialog>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/application/services.h>
#include <haruhi/lib/controller.h>
#include <haruhi/lib/controller_proxy.h>
#include <haruhi/widgets/dial_control.h>
#include <haruhi/widgets/wave_plot.h>


namespace Haruhi {

class Knob;


/**
 * Can't insert it into Knob namespace due to Qt's MOC restrictions.
 */
class KnobProperties: public QDialog
{
	Q_OBJECT

	class CurveWave: public DSP::Wave
	{
	  public:
		CurveWave (Knob* knob);

		Sample
		operator() (Sample phase, Sample frequency) const noexcept override;

	  private:
		Knob* _knob;
	};

  public:
	KnobProperties (Knob* knob, QWidget* parent);

	void
	apply();

  public slots:
	/**
	 * Update plot depending on current knob state.
	 */
	void
	update_plot();

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
	WavePlot*	_curve_plot;
	CurveWave	_curve_wave;
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
		SpinBox (QWidget* parent, Knob* knob, Range<int> user_limit, Range<float> shown_range, int shown_decimals, int step);

		/*
		 * QSpinBox API
		 */

		QValidator::State
		validate (QString&, int&) const override;

		Range<float>
		shown_range() const noexcept;

		int
		shown_decimals() const noexcept;

		bool
		volume_scale() const noexcept;

		float
		volume_scale_exp() const noexcept;

		void
		set_volume_scale (bool setting, float exp = 1.0f);

		void
		set_narrow (bool narrow);

		/**
		 * Detached knob works like a simple QDoubleSpinBox,
		 * that is it does not care about any knob.
		 */
		void
		set_detached (bool setting);

	  protected:
		/*
		 * QSpinBox API
		 */

		QString
		textFromValue (int value) const override;

		int
		valueFromText (QString const& text) const override;

	  private:
		float
		int_to_float (int) const;

		int
		float_to_int (float) const;

	  private:
		Knob*				_knob;
		Range<float>		_shown_range;
		int					_shown_decimals;
		QDoubleValidator*	_validator;
		bool				_detached;
		bool				_volume_scale;
		float				_volume_scale_exp;
	};

	typedef std::map<int, Port*> ContextMenuPortMap;

  public:
	/**
	 * Create Knob.
	 *
	 * \param	parent: Parent widget.
	 * \param	event_port: EventPort coupled with this knob.
	 * \param	controller_param: Parameter to be controlled by this knob.
	 * \param	label: Displayed label.
	 * \param	shown_min, shown_max: Values range shown in spinbox.
	 * \param	step: Change step.
	 * \param	shown_decimals: How many decimal digits should be shown in spinbox.
	 */
	Knob (QWidget* parent, EventPort* event_port, ControllerParam* controller_param, QString const& label,
		  Range<float> shown_range, int step, int shown_decimals);

	/**
	 * Create Knob.
	 * Similar to the previous constructor, but takes shown_min/max, step and decimals params
	 * from the controller_param itself.
	 */
	Knob (QWidget* parent, EventPort* event_port, ControllerParam* controller_param, QString const& label);

	/**
	 * Create Knob. Use external ControllerProxy instead of own one.
	 */
	Knob (QWidget* parent, ControllerProxy* controller_proxy, QString const& label,
		  Range<float> shown_range, int step, int shown_decimals);

	/**
	 * Create Knob. Use external ControllerProxy and min/max/step/decimals params taken from the
	 * controlled parameter itself.
	 */
	Knob (QWidget* parent, ControllerProxy* controller_proxy, QString const& label);

	/**
	 * Returns true if volume scale has been enabled.
	 */
	bool
	volume_scale() const noexcept;

	/**
	 * Returns volume scale exponent.
	 */
	float
	volume_scale_exp() const noexcept;

	/**
	 * Show/hide label.
	 */
	void
	set_label_visible (bool visible);

	/**
	 * Show/hide spinbox.
	 */
	void
	set_spinbox_visible (bool visible);

	/**
	 * Set more narrow size.
	 */
	void
	set_narrow (bool narrow);

	/**
	 * Enables/disables volume scale (shown in dB).
	 * \param	exp is power value, usually M_E.
	 */
	void
	set_volume_scale (bool setting, float exp = 1.0f);

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
	 * ControllerProxy::Widget API
	 */

	bool
	user_override();

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
	/**
	 * Common ctor code.
	 */
	void
	initialize (QString const& label, Range<float> shown_range, int step, int shown_decimals);

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
	learning_state_changed() override;

  protected:
	void
	mousePressEvent (QMouseEvent*) override;

	void
	mouseDoubleClickEvent (QMouseEvent*) override;

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
	start_learning_slot();

	void
	stop_learning_slot();

  signals:
	void
	changed (int);

  private:
	bool				_prevent_recursion			= false;
	QSignalMapper*		_connect_signal_mapper		= nullptr;
	QSignalMapper*		_disconnect_signal_mapper	= nullptr;
	ContextMenuPortMap	_context_menu_port_map;
	int					_action_id; // Helper for generating new IDs for _signal_mapper.
	QColor				_std_text_color;

	// Widgets:
	KnobProperties*		_knob_properties			= nullptr;
	QLabel*				_label						= nullptr;
	DialControl*		_dial_control				= nullptr;
	SpinBox*			_spin_box					= nullptr;
	QMenu*				_context_menu				= nullptr;
	QMenu*				_connect_menu				= nullptr;
	QMenu*				_disconnect_menu			= nullptr;
};


inline
KnobProperties::CurveWave::CurveWave (Knob* knob):
	Wave (true),
	_knob (knob)
{
}


inline Sample
KnobProperties::CurveWave::operator() (Sample phase, Sample) const noexcept
{
	ControllerParam::Adapter const* adapter = _knob->param()->adapter();
	return renormalize (adapter->forward_normalized (phase), adapter->hard_limit, Range<float> {-1.0f, 1.0f});
}


inline Range<float>
Knob::SpinBox::shown_range() const noexcept
{
	return _shown_range;
}


inline int
Knob::SpinBox::shown_decimals() const noexcept
{
	return _shown_decimals;
}


inline bool
Knob::SpinBox::volume_scale() const noexcept
{
	return _volume_scale;
}


inline float
Knob::SpinBox::volume_scale_exp() const noexcept
{
	return _volume_scale_exp;
}


inline void
Knob::SpinBox::set_narrow (bool narrow)
{
	setFixedWidth (font().pointSize() * Services::x_pixels_per_point() * (narrow ? 4.5f : 5.0f));
}


inline void
Knob::SpinBox::set_detached (bool setting)
{
	_detached = setting;
}


inline bool
Knob::volume_scale() const noexcept
{
	return _spin_box->volume_scale();
}


inline float
Knob::volume_scale_exp() const noexcept
{
	return _spin_box->volume_scale_exp();
}


inline void
Knob::set_label_visible (bool visible)
{
	_label->setVisible (visible);
}


inline void
Knob::set_spinbox_visible (bool visible)
{
	_spin_box->setVisible (visible);
}


inline void
Knob::set_narrow (bool narrow)
{
	_spin_box->set_narrow (narrow);
}


inline void
Knob::set_volume_scale (bool setting, float exp)
{
	_spin_box->set_volume_scale (setting, exp);
}


inline bool
Knob::user_override()
{
	return _dial_control->mouse_pressed();
}


inline void
Knob::start_learning_slot()
{
	start_learning();
}


inline void
Knob::stop_learning_slot()
{
	stop_learning();
}

} // namespace Haruhi

#endif

