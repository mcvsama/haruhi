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

// Standard:
#include <cstddef>
#include <cmath>
#include <map>
#include <set>

// Qt:
#include <QtCore/QEvent>
#include <QtGui/QApplication>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QValidator>
#include <QtGui/QCursor>
#include <QtGui/QPushButton>
#include <QtGui/QMouseEvent>
#include <QtGui/QSpinBox>
#include <QtGui/QMenu>

// Haruhi:
#include <haruhi/config.h>
#include <haruhi/haruhi.h>
#include <haruhi/session.h>
#include <haruhi/core/event_buffer.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "knob.h"


KnobProperties::KnobProperties (Knob* knob, QWidget* parent):
	QDialog (parent),
	_knob (knob),
	_smoothing_spinbox (0)
{
	setCaption ("Knob properties");
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	Knob::SpinBox* s = knob->_spin_box;
	ControllerProxy::Config* c = _knob->controller_proxy()->config();

	QVBoxLayout* layout = new QVBoxLayout (this, Config::dialog_margin, Config::spacing);
	layout->setResizeMode (QLayout::Fixed);

	QGridLayout* grid_layout = new QGridLayout (layout, 2, 2, Config::spacing);
	layout->addItem (new QSpacerItem (0, Config::spacing, QSizePolicy::Fixed, QSizePolicy::Fixed));
	QHBoxLayout* buttons_layout = new QHBoxLayout (layout, Config::spacing);

	QLabel* curve_label = new QLabel ("Response curve:", this);
	Knob::SpinBox* curve_spinbox = new Knob::SpinBox (this, _knob, -1000, 1000, -1.0, 1.0, 100, 1);
	curve_spinbox->set_detached (true);
	curve_spinbox->setValue (c->curve * 1000.0);
	curve_spinbox->setFixedWidth (80);
	_curve_spinbox = curve_spinbox;

	QLabel* range_min_label = new QLabel ("Range minimum:", this);
	Knob::SpinBox* user_limit_min_spinbox = new Knob::SpinBox (this, _knob, c->hard_limit_min, c->hard_limit_max, s->show_min(), s->show_max(), s->singleStep(), s->decimals());
	user_limit_min_spinbox->set_detached (true);
	user_limit_min_spinbox->setValue (c->user_limit_min);
	user_limit_min_spinbox->setFixedWidth (80);
	QObject::connect (user_limit_min_spinbox, SIGNAL (valueChanged (int)), this, SLOT (limit_min_updated()));
	_user_limit_min_spinbox = user_limit_min_spinbox;

	QLabel* range_max_label = new QLabel ("Range maximum:", this);
	Knob::SpinBox* user_limit_max_spinbox = new Knob::SpinBox (this, _knob, c->hard_limit_min, c->hard_limit_max, s->show_min(), s->show_max(), s->singleStep(), s->decimals());
	user_limit_max_spinbox->set_detached (true);
	user_limit_max_spinbox->setValue (c->user_limit_max);
	user_limit_max_spinbox->setFixedWidth (80);
	QObject::connect (user_limit_max_spinbox, SIGNAL (valueChanged (int)), this, SLOT (limit_max_updated()));
	_user_limit_max_spinbox = user_limit_max_spinbox;

	QLabel* smoothing_label = 0;
	if (_knob->controller_proxy()->smoothing_parameter())
	{
		smoothing_label = new QLabel ("Smoothing:", this);
		_smoothing_spinbox = new QSpinBox (this);
		_smoothing_spinbox->setMinimum (0);
		_smoothing_spinbox->setMaximum (500);
		_smoothing_spinbox->setSingleStep (5);
		_smoothing_spinbox->setSuffix (" ms");
		_smoothing_spinbox->setSpecialValueText ("Off");
		_smoothing_spinbox->setMinimumWidth (65);
		_smoothing_spinbox->setValue (_knob->controller_proxy()->smoothing_value());
		_smoothing_spinbox->setFixedWidth (80);
	}

	grid_layout->addWidget (curve_label, 0, 0, Qt::AlignLeft);
	grid_layout->addWidget (range_min_label, 1, 0, Qt::AlignLeft);
	grid_layout->addWidget (range_max_label, 2, 0, Qt::AlignLeft);
	grid_layout->addWidget (_curve_spinbox, 0, 1, Qt::AlignRight);
	grid_layout->addWidget (_user_limit_min_spinbox, 1, 1, Qt::AlignRight);
	grid_layout->addWidget (_user_limit_max_spinbox, 2, 1, Qt::AlignRight);
	if (_knob->controller_proxy()->smoothing_parameter())
	{
		grid_layout->addWidget (smoothing_label, 3, 0, Qt::AlignLeft);
		grid_layout->addWidget (_smoothing_spinbox, 3, 1, Qt::AlignRight);
	}

	QPushButton* accept_button = new QPushButton ("&Ok", this);
	accept_button->setDefault (true);
	QPushButton* reject_button = new QPushButton ("&Cancel", this);

	QObject::connect (accept_button, SIGNAL (clicked()), this, SLOT (accept()));
	QObject::connect (reject_button, SIGNAL (clicked()), this, SLOT (reject()));

	buttons_layout->addItem (new QSpacerItem (50, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (accept_button);
	buttons_layout->addWidget (reject_button);

	// Range spinboxes: min must be strictly less than max
	_user_limit_min_spinbox->setMaximum (_user_limit_max_spinbox->maximum() - 1);
	_user_limit_max_spinbox->setMinimum (_user_limit_min_spinbox->minimum() + 1);
}


void
KnobProperties::apply()
{
	int value = _knob->controller_proxy()->value();
	_knob->controller_proxy()->config()->curve = _curve_spinbox->value() / 1000.0;
	_knob->controller_proxy()->config()->user_limit_min = _user_limit_min_spinbox->value();
	_knob->controller_proxy()->config()->user_limit_max = _user_limit_max_spinbox->value();
	_knob->controller_proxy()->set_value (value);
	if (_smoothing_spinbox)
		_knob->controller_proxy()->set_smoothing_value (_smoothing_spinbox->value());
}


void
KnobProperties::limit_min_updated()
{
	if (_user_limit_min_spinbox->value() >= _user_limit_max_spinbox->value())
		_user_limit_max_spinbox->setValue (_user_limit_min_spinbox->value() + _user_limit_min_spinbox->singleStep());
}


void
KnobProperties::limit_max_updated()
{
	if (_user_limit_max_spinbox->value() <= _user_limit_min_spinbox->value())
		_user_limit_min_spinbox->setValue (_user_limit_max_spinbox->value() - _user_limit_max_spinbox->singleStep());
}


Knob::SpinBox::SpinBox (QWidget* parent, Knob* knob, int user_limit_min, int user_limit_max, float show_min, float show_max, int step, int decimals):
	QSpinBox (parent),
	_knob (knob),
	_show_min (show_min),
	_show_max (show_max),
	_decimals (decimals),
	_detached (false)
{
	_validator = new QDoubleValidator (show_min, show_max, decimals, this);
	QSpinBox::setMinimum (user_limit_min);
	QSpinBox::setMaximum (user_limit_max);
	QSpinBox::setSingleStep (step);
	setFixedWidth (55);
}


QValidator::State
Knob::SpinBox::validate (QString& input, int& pos) const
{
	return _validator->validate (input, pos);
}


void
Knob::SpinBox::set_detached (bool setting)
{
	_detached = setting;
}


QString
Knob::SpinBox::textFromValue (int value) const
{
	return QString ("%1").arg (int_to_float (value), 0, 'f', _decimals);
}


int
Knob::SpinBox::valueFromText (QString const& string) const
{
	return float_to_int (string.toFloat());
}


float
Knob::SpinBox::int_to_float (int x) const
{
	int const hard_limit_min = _detached ? minimum() : _knob->controller_proxy()->config()->hard_limit_min;
	int const hard_limit_max = _detached ? maximum() : _knob->controller_proxy()->config()->hard_limit_max;
	float f = renormalize (x, hard_limit_min, hard_limit_max, _show_min, _show_max);
	if (f < 0.0 && f > 0.5 * -std::pow (0.1, _decimals))
		f = 0.0;
	return f;
}


int
Knob::SpinBox::float_to_int (float y) const
{
	int const hard_limit_min = _detached ? minimum() : _knob->controller_proxy()->config()->hard_limit_min;
	int const hard_limit_max = _detached ? maximum() : _knob->controller_proxy()->config()->hard_limit_max;
	return renormalize (y, _show_min, _show_max, hard_limit_min, hard_limit_max);
}


Knob::Knob (QWidget* parent, ControllerProxy* controller_proxy, QString const& label, float show_min, float show_max, int step, int decimals):
	QFrame (parent),
	_controller_proxy (controller_proxy),
	_unit_bay (0),
	_prevent_recursion (false),
	_connect_signal_mapper (0),
	_disconnect_signal_mapper (0),
	_learning (false)
{
	int const hard_limit_min = _controller_proxy->config()->hard_limit_min;
	int const hard_limit_max = _controller_proxy->config()->hard_limit_max;
	int const user_limit_min = _controller_proxy->config()->user_limit_min;
	int const user_limit_max = _controller_proxy->config()->user_limit_max;

	setFrameStyle (QFrame::StyledPanel | QFrame::Raised);
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	_label = new QLabel (label, this);
	_dial_control = new DialControl (this, hard_limit_min, hard_limit_max, _controller_proxy->config()->reverse (_controller_proxy->value()));
	_spin_box = new SpinBox (this, this, user_limit_min, user_limit_max, show_min, show_max, step, decimals);
	_label->setBuddy (_spin_box);
	_context_menu = new QMenu (this);
	_std_text_color = _label->paletteForegroundColor();

	QVBoxLayout* layout = new QVBoxLayout (this, Config::margin, Config::spacing + 2);
	QHBoxLayout* label_layout = new QHBoxLayout (layout, 0);
	QHBoxLayout* dial_layout = new QHBoxLayout (layout, 0);

	label_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
	label_layout->addWidget (_label);
	label_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

	dial_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
	dial_layout->addWidget (_dial_control);
	dial_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

	layout->addWidget (_spin_box);

	QObject::connect (_dial_control, SIGNAL (valueChanged (int)), this, SLOT (dial_changed (int)));
	QObject::connect (_spin_box, SIGNAL (valueChanged (int)), this, SLOT (spin_changed (int)));

	_controller_proxy->set_widget (this);
	schedule_for_update();
}


Knob::~Knob()
{
	_controller_proxy->set_widget (0);
	forget_about_update();
}


void
Knob::read_config()
{
	_spin_box->setMinimum (_controller_proxy->config()->user_limit_min);
	_spin_box->setMaximum (_controller_proxy->config()->user_limit_max);
	schedule_for_update();
}


void
Knob::read()
{
	int value = _controller_proxy->value();
	_prevent_recursion = true;
	_dial_control->setValue (_controller_proxy->config()->reverse (value));
	_spin_box->setValue (value);
	emit changed (value);
	_prevent_recursion = false;
	update_widgets();
}


void
Knob::periodic_update()
{
	read();
}


void
Knob::reset()
{
	_controller_proxy->reset();
	schedule_for_update();
}


void
Knob::configure()
{
	KnobProperties* dialog = new KnobProperties (this, this);
	if (dialog->exec() == KnobProperties::Accepted)
	{
		dialog->apply();
		read_config();
	}
}


void
Knob::start_learning()
{
	_learning = true;
	_unit_bay->session()->event_backend()->start_learning (this, Haruhi::EventBackend::Controller | Haruhi::EventBackend::Pitchbend);
	update_widgets();
}


void
Knob::stop_learning()
{
	_learning = false;
	_unit_bay->session()->event_backend()->stop_learning (this, Haruhi::EventBackend::Controller | Haruhi::EventBackend::Pitchbend);
	update_widgets();
}


void
Knob::update_widgets()
{
	QPalette palette = _label->palette();
	palette.setColor (QPalette::Active, QPalette::WindowText, _learning ? QColor (0x00, 0x11, 0xff) : _std_text_color);
	palette.setColor (QPalette::Inactive, QPalette::WindowText, _learning ? QColor (0x00, 0x11, 0xff) : _std_text_color);
	_label->setPalette (palette);
	update();
}


QMenu*
Knob::create_context_menu()
{
	_action_id = 0;
	_context_menu->clear();

	if (_connect_signal_mapper)
		delete _connect_signal_mapper;
	if (_disconnect_signal_mapper)
		delete _disconnect_signal_mapper;
	_context_menu_port_map.clear();

	_connect_signal_mapper = new QSignalMapper (this);
	QObject::connect (_connect_signal_mapper, SIGNAL (mapped (int)), this, SLOT (connect_port (int)));
	_disconnect_signal_mapper = new QSignalMapper (this);
	QObject::connect (_disconnect_signal_mapper, SIGNAL (mapped (int)), this, SLOT (disconnect_port (int)));

	QPixmap pixmap_for_unit (Config::Icons16::unit());
	QPixmap pixmap_for_port (Config::Icons16::event_output_port());
	QPixmap pixmap_for_port_group (Config::Icons16::port_group());

	_context_menu->addAction ("&Reset", this, SLOT (reset()));
	if (_learning)
		_context_menu->addAction (Config::Icons16::colorpicker(), "Stop learning", this, SLOT (stop_learning()));
	else
		_context_menu->addAction (Config::Icons16::colorpicker(), "&Learn", this, SLOT (start_learning()));
	_context_menu->addAction (Config::Icons16::configure(), "Con&figure", this, SLOT (configure()));

	// Add Connect/Disconnect menu items:
	if (_unit_bay)
	{
		_context_menu->addSeparator();
		_connect_menu = _context_menu->addMenu (Config::Icons16::connect(), "&Connect");
		_disconnect_menu = _context_menu->addMenu (Config::Icons16::disconnect(), "&Disconnect");

		_unit_bay->graph()->lock();

		// Connect AudioBackend:
		QMenu* audio_backend_menu = _connect_menu->addMenu (pixmap_for_unit, QString::fromStdString (_unit_bay->session()->audio_backend()->title()));
		create_connect_menu (audio_backend_menu, _unit_bay->session()->audio_backend(), pixmap_for_port_group, pixmap_for_port);

		// Connect EventBackend:
		QMenu* event_backend_menu = _connect_menu->addMenu (pixmap_for_unit, QString::fromStdString (_unit_bay->session()->event_backend()->title()));
		create_connect_menu (event_backend_menu, _unit_bay->session()->event_backend(), pixmap_for_port_group, pixmap_for_port);

		// Iterate over all Units from UnitBay and create PopupMenus for their EventPorts:
		for (Haruhi::UnitBay::Units::iterator u = _unit_bay->units().begin(); u != _unit_bay->units().end(); ++u)
		{
			QMenu* unit_menu = _connect_menu->addMenu (pixmap_for_unit, QString::fromStdString ((*u)->title()));
			create_connect_menu (unit_menu, *u, pixmap_for_port_group, pixmap_for_port);
		}

		// Iterate over all connected ports and create Disconnect menu:
		Core::Ports const& back_connections = _controller_proxy->event_port()->back_connections();
		if (!back_connections.empty())
		{
			_context_menu->addSeparator();
			_context_menu->addAction ("Disconnect from all", this, SLOT (disconnect_from_all()));
		}
		for (Core::Ports::iterator p = back_connections.begin(); p != back_connections.end(); ++p)
		{
			_action_id += 1;
			QAction* action = _disconnect_menu->addAction (pixmap_for_port,
														   QString::fromStdString ((*p)->unit()->title() + " • " + ((*p)->group() ? (*p)->group()->name() + " • " : "") + (*p)->name()),
														   _disconnect_signal_mapper, SLOT (map()));
			_disconnect_signal_mapper->setMapping (action, _action_id);
			_context_menu_port_map[_action_id] = *p;
		}

		_connect_menu->setEnabled (!_connect_menu->isEmpty());
		_disconnect_menu->setEnabled (!_disconnect_menu->isEmpty());

		_unit_bay->graph()->unlock();
	}

	return _context_menu;
}


void
Knob::create_connect_menu (QMenu* unit_menu, Core::Unit* unit, QPixmap const& pixmap_for_port_group, QPixmap const& pixmap_for_port)
{
	typedef std::vector<Core::Port*> PortsVector;
	typedef std::vector<Core::PortGroup*> GroupsVector;
	typedef std::map<Core::PortGroup*, PortsVector> GroupsMap;

	GroupsMap groups;
	PortsVector ports;

	// Collect ports and groups:
	for (Core::Ports::iterator p = unit->outputs().begin(); p != unit->outputs().end(); ++p)
	{
		Core::EventPort* ep = dynamic_cast<Core::EventPort*> (*p);
		if (ep)
		{
			if (ep->group())
				groups[ep->group()].push_back (ep);
			else
				ports.push_back (ep);
		}
	}

	// Add group items and submenus:
	for (GroupsMap::iterator g = groups.begin(); g != groups.end(); ++g)
	{
		QMenu* group_menu = unit_menu->addMenu (pixmap_for_port_group, QString::fromStdString (g->first->name()));
		std::sort (g->second.begin(), g->second.end(), Core::Port::CompareByName());
		for (PortsVector::iterator p = g->second.begin(); p != g->second.end(); ++p)
		{
			_action_id += 1;
			QAction* action = group_menu->addAction (pixmap_for_port, QString::fromStdString ((*p)->name()), _connect_signal_mapper, SLOT (map()));
			action->setEnabled (!(*p)->connected_to (_controller_proxy->event_port()));
			_connect_signal_mapper->setMapping (action, _action_id);
			_context_menu_port_map[_action_id] = *p;
		}
	}

	// Add port items:
	std::sort (ports.begin(), ports.end(), Core::Port::CompareByName());
	for (PortsVector::iterator p = ports.begin(); p != ports.end(); ++p)
	{
		_action_id += 1;
		QAction* action = unit_menu->addAction (pixmap_for_port, QString::fromStdString ((*p)->name()), _connect_signal_mapper, SLOT (map()));
		action->setEnabled (!(*p)->connected_to (_controller_proxy->event_port()));
		_connect_signal_mapper->setMapping (action, _action_id);
		_context_menu_port_map[_action_id] = *p;
	}

	unit_menu->menuAction()->setVisible (!unit_menu->isEmpty());
}


void
Knob::learned_port (Haruhi::EventBackend::EventTypes, Core::EventPort* event_port)
{
	if (_unit_bay)
	{
		_learning = false;
		_unit_bay->graph()->lock();
		event_port->connect_to (_controller_proxy->event_port());
		_unit_bay->graph()->unlock();
		schedule_for_update();
	}
}


void
Knob::mousePressEvent (QMouseEvent* event)
{
	if (event->button() == Qt::RightButton)
	{
		event->accept();
		_context_menu = create_context_menu();
		_context_menu->popup (QCursor::pos());
	}
	else
		event->ignore();
}


void
Knob::mouseDoubleClickEvent (QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		event->accept();
		configure();
	}
	else
		event->ignore();
}


void
Knob::dial_changed (int value)
{
	if (_prevent_recursion)
		return;
	_controller_proxy->set_value (_controller_proxy->config()->forward (value));
	schedule_for_update();
}


void
Knob::spin_changed (int value)
{
	if (_prevent_recursion)
		return;
	_controller_proxy->set_value (value);
	schedule_for_update();
}


void
Knob::connect_port (int action_id)
{
	if (_unit_bay)
	{
		_unit_bay->graph()->lock();
		ContextMenuPortMap::iterator a = _context_menu_port_map.find (action_id);
		// FIXME If port is deleted between menu popup and action exec, connect_to() will be executed on singular object.
		if (a != _context_menu_port_map.end())
			a->second->connect_to (_controller_proxy->event_port());
		_unit_bay->graph()->unlock();
	}
}


void
Knob::disconnect_port (int action_id)
{
	if (_unit_bay)
	{
		_unit_bay->graph()->lock();
		ContextMenuPortMap::iterator a = _context_menu_port_map.find (action_id);
		// FIXME If port is deleted between menu popup and action exec, connect_to() will be executed on singular object.
		if (a != _context_menu_port_map.end())
			a->second->disconnect_from (_controller_proxy->event_port());
		_unit_bay->graph()->unlock();
	}
}


void
Knob::disconnect_from_all()
{
	if (_unit_bay)
	{
		_unit_bay->graph()->lock();
		_controller_proxy->event_port()->disconnect();
		_unit_bay->graph()->unlock();
	}
}

