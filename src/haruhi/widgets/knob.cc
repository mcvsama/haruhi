/* vim:ts=4
 *
 * Copyleft 2008…2011  Michał Gawron
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
#include <haruhi/config/all.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/graph/audio_backend.h>
#include <haruhi/graph/event_backend.h>
#include <haruhi/graph/event_buffer.h>
#include <haruhi/session/unit_bay.h>
#include <haruhi/utility/atomic.h>
#include <haruhi/utility/numeric.h>

// Local:
#include "knob.h"


namespace Haruhi {

KnobProperties::KnobProperties (Knob* knob, QWidget* parent):
	QDialog (parent),
	_knob (knob)
{
	setCaption ("Knob properties");
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	Knob::SpinBox* s = knob->_spin_box;
	ControllerParam::Adapter* a = _knob->controller_proxy()->param()->adapter();

	QLabel* curve_label = new QLabel ("Response curve:", this);
	Knob::SpinBox* curve_spinbox = new Knob::SpinBox (this, _knob, -1000, 1000, -1.0, 1.0, 1, 100);
	curve_spinbox->set_detached (true);
	curve_spinbox->setValue (a->curve * 1000.0);
	curve_spinbox->setFixedWidth (80);
	_curve_spinbox = curve_spinbox;

	QLabel* range_min_label = new QLabel ("Range minimum:", this);
	Knob::SpinBox* user_limit_min_spinbox = new Knob::SpinBox (this, _knob, a->hard_limit_min, a->hard_limit_max, s->shown_min(), s->shown_max(), s->shown_decimals(), s->singleStep());
	user_limit_min_spinbox->set_detached (true);
	user_limit_min_spinbox->setValue (a->user_limit_min);
	user_limit_min_spinbox->setFixedWidth (80);
	user_limit_min_spinbox->set_volume_scale (knob->volume_scale(), knob->volume_scale_exp());
	QObject::connect (user_limit_min_spinbox, SIGNAL (valueChanged (int)), this, SLOT (limit_min_updated()));
	_user_limit_min_spinbox = user_limit_min_spinbox;

	QLabel* range_max_label = new QLabel ("Range maximum:", this);
	Knob::SpinBox* user_limit_max_spinbox = new Knob::SpinBox (this, _knob, a->hard_limit_min, a->hard_limit_max, s->shown_min(), s->shown_max(), s->shown_decimals(), s->singleStep());
	user_limit_max_spinbox->set_detached (true);
	user_limit_max_spinbox->setValue (a->user_limit_max);
	user_limit_max_spinbox->setFixedWidth (80);
	user_limit_max_spinbox->set_volume_scale (knob->volume_scale(), knob->volume_scale_exp());
	QObject::connect (user_limit_max_spinbox, SIGNAL (valueChanged (int)), this, SLOT (limit_max_updated()));
	_user_limit_max_spinbox = user_limit_max_spinbox;

	QPushButton* accept_button = new QPushButton ("&Ok", this);
	accept_button->setDefault (true);
	QPushButton* reject_button = new QPushButton ("&Cancel", this);

	QObject::connect (accept_button, SIGNAL (clicked()), this, SLOT (accept()));
	QObject::connect (reject_button, SIGNAL (clicked()), this, SLOT (reject()));

	// Layouts:

	QGridLayout* grid_layout = new QGridLayout();
	grid_layout->setSpacing (Config::Spacing);
	grid_layout->addWidget (curve_label, 0, 0, Qt::AlignLeft);
	grid_layout->addWidget (range_min_label, 1, 0, Qt::AlignLeft);
	grid_layout->addWidget (range_max_label, 2, 0, Qt::AlignLeft);
	grid_layout->addWidget (_curve_spinbox, 0, 1, Qt::AlignRight);
	grid_layout->addWidget (_user_limit_min_spinbox, 1, 1, Qt::AlignRight);
	grid_layout->addWidget (_user_limit_max_spinbox, 2, 1, Qt::AlignRight);

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::Spacing);
	buttons_layout->addItem (new QSpacerItem (50, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (accept_button);
	buttons_layout->addWidget (reject_button);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::DialogMargin);
	layout->setSpacing (Config::Spacing);
	layout->setResizeMode (QLayout::Fixed);
	layout->addLayout (grid_layout);
	layout->addItem (new QSpacerItem (0, Config::Spacing, QSizePolicy::Fixed, QSizePolicy::Fixed));
	layout->addLayout (buttons_layout);

	// Range spinboxes: min must be strictly less than max
	_user_limit_min_spinbox->setMaximum (_user_limit_max_spinbox->maximum() - 1);
	_user_limit_max_spinbox->setMinimum (_user_limit_min_spinbox->minimum() + 1);
}


void
KnobProperties::apply()
{
	int value = _knob->param()->get();
	_knob->controller_proxy()->param()->adapter()->curve = _curve_spinbox->value() / 1000.0;
	_knob->controller_proxy()->param()->adapter()->user_limit_min = _user_limit_min_spinbox->value();
	_knob->controller_proxy()->param()->adapter()->user_limit_max = _user_limit_max_spinbox->value();
	_knob->controller_proxy()->set_absolute_value (value);
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


Knob::SpinBox::SpinBox (QWidget* parent, Knob* knob, int user_limit_min, int user_limit_max, float shown_min, float shown_max, int shown_decimals, int step):
	QSpinBox (parent),
	_knob (knob),
	_shown_min (shown_min),
	_shown_max (shown_max),
	_shown_decimals (shown_decimals),
	_detached (false),
	_volume_scale (false)
{
	_validator = new QDoubleValidator (shown_min, shown_max, shown_decimals, this);
	QSpinBox::setMinimum (user_limit_min);
	QSpinBox::setMaximum (user_limit_max);
	QSpinBox::setSingleStep (step);
	setFixedWidth (font().pointSize() * 5);
}


QValidator::State
Knob::SpinBox::validate (QString& input, int& pos) const
{
	return _validator->validate (input, pos);
}


void
Knob::SpinBox::set_volume_scale (bool setting, float exp)
{
	_volume_scale = setting;
	_volume_scale_exp = exp;
	// Update does not call textFromValue, so it does not update text in spinbox:
	setValue (value());
}


QString
Knob::SpinBox::textFromValue (int value) const
{
	if (_volume_scale)
	{
		int const hard_limit_min = _detached ? minimum() : _knob->controller_proxy()->param()->adapter()->hard_limit_min;
		int const hard_limit_max = _detached ? maximum() : _knob->controller_proxy()->param()->adapter()->hard_limit_max;
		double x = static_cast<double> (value - hard_limit_min) / static_cast<double> (hard_limit_max - hard_limit_min);
		return QString ("%1").arg (20.0 * std::log10 (std::pow (x, _volume_scale_exp)), 0, 'f', 1);
	}
	else
		return QString ("%1").arg (int_to_float (value), 0, 'f', _shown_decimals);
}


int
Knob::SpinBox::valueFromText (QString const& string) const
{
	if (_volume_scale)
	{
		int const hard_limit_min = _detached ? minimum() : _knob->controller_proxy()->param()->adapter()->hard_limit_min;
		int const hard_limit_max = _detached ? maximum() : _knob->controller_proxy()->param()->adapter()->hard_limit_max;
		return std::pow (std::pow (10.0, string.toFloat() / 20.0), 1.0 / _volume_scale_exp) * static_cast<double> (hard_limit_max - hard_limit_min);
	}
	else
		return float_to_int (string.toFloat());
}


float
Knob::SpinBox::int_to_float (int x) const
{
	int const hard_limit_min = _detached ? minimum() : _knob->controller_proxy()->param()->adapter()->hard_limit_min;
	int const hard_limit_max = _detached ? maximum() : _knob->controller_proxy()->param()->adapter()->hard_limit_max;

	float f = renormalize (x, hard_limit_min, hard_limit_max, _shown_min, _shown_max);
	if (f < 0.0 && f > 0.5 * -std::pow (0.1, _shown_decimals))
		f = 0.0;
	return f;
}


int
Knob::SpinBox::float_to_int (float y) const
{
	int const hard_limit_min = _detached ? minimum() : _knob->controller_proxy()->param()->adapter()->hard_limit_min;
	int const hard_limit_max = _detached ? maximum() : _knob->controller_proxy()->param()->adapter()->hard_limit_max;

	return renormalize (y, _shown_min, _shown_max, hard_limit_min, hard_limit_max);
}


Knob::Knob (QWidget* parent, EventPort* event_port, ControllerParam* controller_param,
			QString const& label, float shown_min, float shown_max, int step, int shown_decimals):
	QFrame (parent),
	Controller (event_port, controller_param)
{
	initialize (label, shown_min, shown_max, shown_decimals, step);
}


Knob::Knob (QWidget* parent, EventPort* event_port, ControllerParam* controller_param, QString const& label):
	QFrame (parent),
	Controller (event_port, controller_param)
{
	initialize (label, controller_param->shown_min(), controller_param->shown_max(), controller_param->shown_decimals(), controller_param->step());
}


Knob::Knob (QWidget* parent, ControllerProxy* controller_proxy,
			QString const& label, float shown_min, float shown_max, int step, int shown_decimals):
	QFrame (parent),
	Controller (controller_proxy)
{
	initialize (label, shown_min, shown_max, shown_decimals, step);
}


Knob::Knob (QWidget* parent, ControllerProxy* controller_proxy, QString const& label):
	QFrame (parent),
	Controller (controller_proxy)
{
	ControllerParam* controller_param = controller_proxy->param();
	initialize (label, controller_param->shown_min(), controller_param->shown_max(), controller_param->shown_decimals(), controller_param->step());
}


void
Knob::read_config()
{
	_spin_box->setMinimum (controller_proxy()->param()->adapter()->user_limit_min);
	_spin_box->setMaximum (controller_proxy()->param()->adapter()->user_limit_max);
	schedule_for_update();
}


void
Knob::read()
{
	int value = controller_proxy()->param()->get();
	_prevent_recursion = true;
	_dial_control->setValue (controller_proxy()->param()->adapter()->reverse (value));
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
	controller_proxy()->reset_value();
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
Knob::initialize (QString const& label, float shown_min, float shown_max, int shown_decimals, int step)
{
	_prevent_recursion = false;
	_connect_signal_mapper = 0;
	_disconnect_signal_mapper = 0;

	int const hard_limit_min = controller_proxy()->param()->adapter()->hard_limit_min;
	int const hard_limit_max = controller_proxy()->param()->adapter()->hard_limit_max;
	int const user_limit_min = controller_proxy()->param()->adapter()->user_limit_min;
	int const user_limit_max = controller_proxy()->param()->adapter()->user_limit_max;

	setFrameStyle (QFrame::StyledPanel | QFrame::Raised);
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	_dial_control = new DialControl (this, hard_limit_min, hard_limit_max, controller_proxy()->param()->adapter()->reverse (controller_proxy()->param()->get()));
	_spin_box = new SpinBox (this, this, user_limit_min, user_limit_max, shown_min, shown_max, shown_decimals, step);
	_label = new QLabel (label, this);
	_label->setBuddy (_spin_box);
	_label->setTextFormat (Qt::PlainText);
	_context_menu = new QMenu (this);
	_std_text_color = _label->paletteForegroundColor();

	QObject::connect (_dial_control, SIGNAL (valueChanged (int)), this, SLOT (dial_changed (int)));
	QObject::connect (_spin_box, SIGNAL (valueChanged (int)), this, SLOT (spin_changed (int)));

	// Layouts:

	QHBoxLayout* label_layout = new QHBoxLayout();
	label_layout->setSpacing (0);
	label_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
	label_layout->addWidget (_label);
	label_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

	QHBoxLayout* dial_layout = new QHBoxLayout();
	dial_layout->setSpacing (0);
	dial_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
	dial_layout->addWidget (_dial_control);
	dial_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::Margin);
	layout->setSpacing (Config::Spacing + 2);
	layout->addLayout (label_layout);
	layout->addLayout (dial_layout);
	layout->addWidget (_spin_box, 0, Qt::AlignCenter);
}


void
Knob::update_widgets()
{
	QPalette palette = _label->palette();
	palette.setColor (QPalette::Active, QPalette::WindowText, learning() ? QColor (0x00, 0x11, 0xff) : _std_text_color);
	palette.setColor (QPalette::Inactive, QPalette::WindowText, learning() ? QColor (0x00, 0x11, 0xff) : _std_text_color);
	_label->setPalette (palette);
	update();
}


QMenu*
Knob::create_context_menu()
{
	_action_id = 0;
	_context_menu->clear();

	delete _connect_signal_mapper;
	delete _disconnect_signal_mapper;
	_context_menu_port_map.clear();

	_connect_signal_mapper = new QSignalMapper (this);
	QObject::connect (_connect_signal_mapper, SIGNAL (mapped (int)), this, SLOT (connect_port (int)));
	_disconnect_signal_mapper = new QSignalMapper (this);
	QObject::connect (_disconnect_signal_mapper, SIGNAL (mapped (int)), this, SLOT (disconnect_port (int)));

	QPixmap pixmap_for_unit (Resources::Icons16::unit());
	QPixmap pixmap_for_port (Resources::Icons16::event_output_port());
	QPixmap pixmap_for_port_group (Resources::Icons16::port_group());

	_context_menu->addAction ("&Reset", this, SLOT (reset()));
	if (learning())
		_context_menu->addAction (Resources::Icons16::colorpicker(), "Stop learning", this, SLOT (stop_learning_slot()));
	else
		_context_menu->addAction (Resources::Icons16::colorpicker(), "&Learn", this, SLOT (start_learning_slot()));
	_context_menu->addAction (Resources::Icons16::configure(), "Con&figure", this, SLOT (configure()));

	// Add Connect/Disconnect menu items:
	if (unit_bay() && event_port())
	{
		_context_menu->addSeparator();
		_connect_menu = _context_menu->addMenu (Resources::Icons16::connect(), "&Connect");
		_disconnect_menu = _context_menu->addMenu (Resources::Icons16::disconnect(), "&Disconnect");

		Graph* graph = unit_bay()->graph();

		graph->lock();

		// Connect AudioBackend:
		QMenu* audio_backend_menu = _connect_menu->addMenu (pixmap_for_unit, QString::fromStdString (graph->audio_backend()->title()));
		create_connect_menu (audio_backend_menu, graph->audio_backend(), pixmap_for_port_group, pixmap_for_port);

		// Connect EventBackend:
		QMenu* event_backend_menu = _connect_menu->addMenu (pixmap_for_unit, QString::fromStdString (graph->event_backend()->title()));
		create_connect_menu (event_backend_menu, graph->event_backend(), pixmap_for_port_group, pixmap_for_port);

		// Iterate over all Units from UnitBay and create PopupMenus for their EventPorts:
		for (UnitBay::Units::iterator u = unit_bay()->units().begin(); u != unit_bay()->units().end(); ++u)
		{
			QMenu* unit_menu = _connect_menu->addMenu (pixmap_for_unit, QString::fromStdString ((*u)->title()));
			create_connect_menu (unit_menu, *u, pixmap_for_port_group, pixmap_for_port);
		}

		// Iterate over all connected ports and create Disconnect menu:
		Ports const& back_connections = event_port()->back_connections();
		if (!back_connections.empty())
		{
			_context_menu->addSeparator();
			_context_menu->addAction ("Disconnect from all", this, SLOT (disconnect_from_all()));
		}
		for (Ports::iterator p = back_connections.begin(); p != back_connections.end(); ++p)
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

		unit_bay()->graph()->unlock();
	}

	return _context_menu;
}


void
Knob::create_connect_menu (QMenu* unit_menu, Unit* unit, QPixmap const& pixmap_for_port_group, QPixmap const& pixmap_for_port)
{
	typedef std::vector<Port*> PortsVector;
	typedef std::vector<PortGroup*> GroupsVector;
	typedef std::map<PortGroup*, PortsVector> GroupsMap;

	GroupsMap groups;
	PortsVector ports;

	// Collect ports and groups:
	for (Ports::iterator p = unit->outputs().begin(); p != unit->outputs().end(); ++p)
	{
		EventPort* ep = dynamic_cast<EventPort*> (*p);
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
		std::sort (g->second.begin(), g->second.end(), Port::CompareByName());
		for (PortsVector::iterator p = g->second.begin(); p != g->second.end(); ++p)
		{
			_action_id += 1;
			QAction* action = group_menu->addAction (pixmap_for_port, QString::fromStdString ((*p)->name()), _connect_signal_mapper, SLOT (map()));
			action->setEnabled (!(*p)->connected_to (event_port()));
			_connect_signal_mapper->setMapping (action, _action_id);
			_context_menu_port_map[_action_id] = *p;
		}
	}

	// Add port items:
	std::sort (ports.begin(), ports.end(), Port::CompareByName());
	for (PortsVector::iterator p = ports.begin(); p != ports.end(); ++p)
	{
		_action_id += 1;
		QAction* action = unit_menu->addAction (pixmap_for_port, QString::fromStdString ((*p)->name()), _connect_signal_mapper, SLOT (map()));
		action->setEnabled (!(*p)->connected_to (event_port()));
		_connect_signal_mapper->setMapping (action, _action_id);
		_context_menu_port_map[_action_id] = *p;
	}

	unit_menu->menuAction()->setVisible (!unit_menu->isEmpty());
}


void
Knob::learning_state_changed()
{
	schedule_for_update();
	update_widgets();
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
	controller_proxy()->set_value_from_widget (value);
}


void
Knob::spin_changed (int value)
{
	if (_prevent_recursion)
		return;
	controller_proxy()->set_absolute_value (value);
}


void
Knob::connect_port (int action_id)
{
	if (unit_bay())
	{
		unit_bay()->graph()->lock();
		ContextMenuPortMap::iterator a = _context_menu_port_map.find (action_id);
		// FIXME If port is deleted between menu popup and action exec, connect_to() will be executed on singular object.
		if (a != _context_menu_port_map.end())
			a->second->connect_to (event_port());
		unit_bay()->graph()->unlock();
	}
}


void
Knob::disconnect_port (int action_id)
{
	if (unit_bay())
	{
		unit_bay()->graph()->lock();
		ContextMenuPortMap::iterator a = _context_menu_port_map.find (action_id);
		// FIXME If port is deleted between menu popup and action exec, connect_to() will be executed on singular object.
		if (a != _context_menu_port_map.end())
			a->second->disconnect_from (event_port());
		unit_bay()->graph()->unlock();
	}
}


void
Knob::disconnect_from_all()
{
	if (unit_bay())
	{
		unit_bay()->graph()->lock();
		event_port()->disconnect();
		unit_bay()->graph()->unlock();
	}
}

} // namespace Haruhi

