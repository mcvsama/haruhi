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
#include <haruhi/widgets/plot_frame.h>

// Local:
#include "knob.h"


namespace Haruhi {

KnobProperties::KnobProperties (Knob* knob, QWidget* parent):
	QDialog (parent),
	_knob (knob),
	_curve_wave (knob)
{
	setWindowTitle ("Knob properties");
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	Knob::SpinBox* s = knob->_spin_box;
	ControllerParam::Adapter* a = _knob->controller_proxy()->param()->adapter();

	QLabel* curve_label = new QLabel ("Response curve:", this);
	Knob::SpinBox* curve_spinbox = new Knob::SpinBox (this, _knob, { -1000, 1000 }, { -1.0, 1.0 }, 1, 100);
	curve_spinbox->set_detached (true);
	curve_spinbox->setValue (a->curve * 1000.0);
	curve_spinbox->setFixedWidth (80);
	QObject::connect (curve_spinbox, SIGNAL (valueChanged (int)), this, SLOT (update_plot()));
	_curve_spinbox = curve_spinbox;

	QLabel* range_min_label = new QLabel ("Range minimum:", this);
	Knob::SpinBox* user_limit_min_spinbox = new Knob::SpinBox (this, _knob, a->hard_limit, s->shown_range(), s->shown_decimals(), s->singleStep());
	user_limit_min_spinbox->set_detached (true);
	user_limit_min_spinbox->setValue (a->user_limit.min());
	user_limit_min_spinbox->setFixedWidth (80);
	user_limit_min_spinbox->set_volume_scale (knob->volume_scale(), knob->volume_scale_exp());
	QObject::connect (user_limit_min_spinbox, SIGNAL (valueChanged (int)), this, SLOT (limit_min_updated()));
	QObject::connect (user_limit_min_spinbox, SIGNAL (valueChanged (int)), this, SLOT (update_plot()));
	_user_limit_min_spinbox = user_limit_min_spinbox;

	QLabel* range_max_label = new QLabel ("Range maximum:", this);
	Knob::SpinBox* user_limit_max_spinbox = new Knob::SpinBox (this, _knob, a->hard_limit, s->shown_range(), s->shown_decimals(), s->singleStep());
	user_limit_max_spinbox->set_detached (true);
	user_limit_max_spinbox->setValue (a->user_limit.max());
	user_limit_max_spinbox->setFixedWidth (80);
	user_limit_max_spinbox->set_volume_scale (knob->volume_scale(), knob->volume_scale_exp());
	QObject::connect (user_limit_max_spinbox, SIGNAL (valueChanged (int)), this, SLOT (limit_max_updated()));
	QObject::connect (user_limit_max_spinbox, SIGNAL (valueChanged (int)), this, SLOT (update_plot()));
	_user_limit_max_spinbox = user_limit_max_spinbox;

	_curve_plot = new WavePlot (this);
	_curve_plot->assign_wave (&_curve_wave, true, true, false);
	_curve_plot->set_phase_marker_enabled (true);

	PlotFrame* plot_frame = new PlotFrame (this);
	plot_frame->set_widget (_curve_plot);

	QPushButton* accept_button = new QPushButton ("&Ok", this);
	accept_button->setDefault (true);
	QPushButton* reject_button = new QPushButton ("&Cancel", this);

	QObject::connect (accept_button, SIGNAL (clicked()), this, SLOT (accept()));
	QObject::connect (reject_button, SIGNAL (clicked()), this, SLOT (reject()));

	// Layouts:

	QGridLayout* grid_layout = new QGridLayout();
	grid_layout->setSpacing (Config::spacing());
	grid_layout->addWidget (plot_frame, 0, 0, 3, 1);
	grid_layout->addItem (new QSpacerItem (Config::spacing(), 0, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 1);
	grid_layout->addWidget (curve_label, 0, 2, Qt::AlignLeft);
	grid_layout->addWidget (range_min_label, 1, 2, Qt::AlignLeft);
	grid_layout->addWidget (range_max_label, 2, 2, Qt::AlignLeft);
	grid_layout->addWidget (_curve_spinbox, 0, 3, Qt::AlignRight);
	grid_layout->addWidget (_user_limit_min_spinbox, 1, 3, Qt::AlignRight);
	grid_layout->addWidget (_user_limit_max_spinbox, 2, 3, Qt::AlignRight);

	QHBoxLayout* buttons_layout = new QHBoxLayout();
	buttons_layout->setSpacing (Config::spacing());
	buttons_layout->addItem (new QSpacerItem (50, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	buttons_layout->addWidget (accept_button);
	buttons_layout->addWidget (reject_button);

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (Config::dialog_margin());
	layout->setSpacing (Config::spacing());
	layout->addLayout (grid_layout);
	layout->addItem (new QSpacerItem (0, Config::spacing(), QSizePolicy::Fixed, QSizePolicy::Fixed));
	layout->addLayout (buttons_layout);

	layout->activate();
	plot_frame->setFixedWidth (plot_frame->height());

	// Range spinboxes: min must be strictly less than max
	_user_limit_min_spinbox->setMaximum (_user_limit_max_spinbox->maximum() - 1);
	_user_limit_max_spinbox->setMinimum (_user_limit_min_spinbox->minimum() + 1);

	update_plot();
}


void
KnobProperties::apply()
{
	int value = _knob->param()->get();
	_knob->controller_proxy()->param()->adapter()->curve = _curve_spinbox->value() / 1000.0;
	_knob->controller_proxy()->param()->adapter()->user_limit.set_min (_user_limit_min_spinbox->value());
	_knob->controller_proxy()->param()->adapter()->user_limit.set_max (_user_limit_max_spinbox->value());
	_knob->controller_proxy()->set_absolute_value (value);
}


void
KnobProperties::update_plot()
{
	apply();
	ControllerParam* param = _knob->controller_proxy()->param();
	_curve_plot->set_phase_marker_position (renormalize (param->adapter()->reverse (param->get()),
														 param->range(), { 0.0f, 1.0f }));
	_curve_plot->plot_shape();
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


Knob::SpinBox::SpinBox (QWidget* parent, Knob* knob, Range<int> user_limit, Range<float> shown_range, int shown_decimals, int step):
	QSpinBox (parent),
	_knob (knob),
	_shown_range (shown_range),
	_shown_decimals (shown_decimals),
	_detached (false),
	_volume_scale (false)
{
	_validator = new QDoubleValidator (_shown_range.min(), _shown_range.max(), shown_decimals, this);
	QSpinBox::setMinimum (user_limit.min());
	QSpinBox::setMaximum (user_limit.max());
	QSpinBox::setSingleStep (step);
	set_narrow (false);
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
		Range<int> const hard_limit = _detached ? Range<int> {minimum(), maximum()} : _knob->controller_proxy()->param()->adapter()->hard_limit;
		double x = static_cast<double> (value - hard_limit.min()) / static_cast<double> (hard_limit.extent());
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
		Range<int> const hard_limit = _detached ? Range<int> {minimum(), maximum()} : _knob->controller_proxy()->param()->adapter()->hard_limit;
		return std::pow (std::pow (10.0, string.toFloat() / 20.0), 1.0 / _volume_scale_exp) * static_cast<double> (hard_limit.extent());
	}
	else
		return float_to_int (string.toFloat());
}


float
Knob::SpinBox::int_to_float (int x) const
{
	Range<int> const hard_limit = _detached ? Range<int> {minimum(), maximum()} : _knob->controller_proxy()->param()->adapter()->hard_limit;
	float f = renormalize (x, hard_limit, _shown_range);
	if (f < 0.0 && f > 0.5 * -std::pow (0.1, _shown_decimals))
		f = 0.0;
	return f;
}


int
Knob::SpinBox::float_to_int (float y) const
{
	Range<int> const hard_limit = _detached ? Range<int> {minimum(), maximum()} : _knob->controller_proxy()->param()->adapter()->hard_limit;
	return renormalize (y, _shown_range, hard_limit);
}


Knob::Knob (QWidget* parent, EventPort* event_port, ControllerParam* controller_param, QString const& label,
			Range<float> shown_range, int step, int shown_decimals):
	QFrame (parent),
	Controller (event_port, controller_param)
{
	initialize (label, shown_range, step, shown_decimals);
}


Knob::Knob (QWidget* parent, EventPort* event_port, ControllerParam* controller_param, QString const& label):
	Knob (parent, event_port, controller_param, label,
		  controller_param->shown_range(), controller_param->step(), controller_param->shown_decimals())
{
}


Knob::Knob (QWidget* parent, ControllerProxy* controller_proxy, QString const& label,
			Range<float> shown_range, int step, int shown_decimals):
	QFrame (parent),
	Controller (controller_proxy)
{
	initialize (label, shown_range, step, shown_decimals);
}


Knob::Knob (QWidget* parent, ControllerProxy* controller_proxy, QString const& label):
	Knob (parent, controller_proxy, label,
		  controller_proxy->param()->shown_range(), controller_proxy->param()->step(), controller_proxy->param()->shown_decimals())
{
}


void
Knob::read_config()
{
	_spin_box->setMinimum (controller_proxy()->param()->adapter()->user_limit.min());
	_spin_box->setMaximum (controller_proxy()->param()->adapter()->user_limit.max());
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
	_knob_properties = new KnobProperties (this, this);
	if (_knob_properties->exec() == KnobProperties::Accepted)
	{
		_knob_properties->apply();
		read_config();
	}
	//TODO restore original settings if user clicked Rejected
	//TODO add icons to OK and Cancel
	_knob_properties = nullptr;
}


void
Knob::initialize (QString const& label, Range<float> shown_range, int step, int shown_decimals)
{
	Range<int> const& hard_limit = controller_proxy()->param()->adapter()->hard_limit;
	Range<int> const& user_limit = controller_proxy()->param()->adapter()->user_limit;

	setFrameStyle (QFrame::StyledPanel | QFrame::Raised);
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

	_dial_control = new DialControl (this, hard_limit, controller_proxy()->param()->adapter()->reverse (controller_proxy()->param()->get()));
	_dial_control->set_ring_visible (true);
	_dial_control->set_center_value (controller_proxy()->param()->center_value());
	_spin_box = new SpinBox (this, this, user_limit, shown_range, shown_decimals, step);
	_label = new QLabel (label, this);
	_label->setBuddy (_spin_box);
	_label->setTextFormat (Qt::PlainText);
	_context_menu = new QMenu (this);
	_std_text_color = _label->palette().color (QPalette::WindowText);

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
	layout->setMargin (Config::margin());
	layout->setSpacing (2_screen_mm);
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
	// If KnobProperties is opened, update it about current value, too:
	if (_knob_properties)
		_knob_properties->update_plot();
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
		Mutex::Lock lock (*graph);

		// Connect AudioBackend:
		QMenu* audio_backend_menu = _connect_menu->addMenu (pixmap_for_unit, QString::fromStdString (graph->audio_backend()->title()));
		create_connect_menu (audio_backend_menu, graph->audio_backend(), pixmap_for_port_group, pixmap_for_port);

		// Connect EventBackend:
		QMenu* event_backend_menu = _connect_menu->addMenu (pixmap_for_unit, QString::fromStdString (graph->event_backend()->title()));
		create_connect_menu (event_backend_menu, graph->event_backend(), pixmap_for_port_group, pixmap_for_port);

		// Iterate over all Units from UnitBay and create PopupMenus for their EventPorts:
		for (Unit* u: unit_bay()->units())
		{
			QMenu* unit_menu = _connect_menu->addMenu (pixmap_for_unit, QString::fromStdString (u->title()));
			create_connect_menu (unit_menu, u, pixmap_for_port_group, pixmap_for_port);
		}

		// Iterate over all connected ports and create Disconnect menu:
		Ports const& back_connections = event_port()->back_connections();
		if (!back_connections.empty())
		{
			_context_menu->addSeparator();
			_context_menu->addAction ("Disconnect from all", this, SLOT (disconnect_from_all()));
		}
		for (Port* p: back_connections)
		{
			_action_id += 1;
			QAction* action = _disconnect_menu->addAction (pixmap_for_port,
														   QString::fromStdString (p->unit()->title() + " • " + (p->group() ? p->group()->name() + " • " : "") + p->name()),
														   _disconnect_signal_mapper, SLOT (map()));
			_disconnect_signal_mapper->setMapping (action, _action_id);
			_context_menu_port_map[_action_id] = p;
		}

		_connect_menu->setEnabled (!_connect_menu->isEmpty());
		_disconnect_menu->setEnabled (!_disconnect_menu->isEmpty());
	}

	return _context_menu;
}


void
Knob::create_connect_menu (QMenu* unit_menu, Unit* unit, QPixmap const& pixmap_for_port_group, QPixmap const& pixmap_for_port)
{
	typedef std::vector<Port*> PortsVector;
	typedef std::map<PortGroup*, PortsVector> GroupsMap;

	GroupsMap groups;
	PortsVector ports;

	// Collect ports and groups:
	for (Port* p: unit->outputs())
	{
		EventPort* ep = dynamic_cast<EventPort*> (p);
		if (ep)
		{
			if (ep->group())
				groups[ep->group()].push_back (ep);
			else
				ports.push_back (ep);
		}
	}

	// Add group items and submenus:
	for (auto& g: groups)
	{
		QMenu* group_menu = unit_menu->addMenu (pixmap_for_port_group, QString::fromStdString (g.first->name()));
		std::sort (g.second.begin(), g.second.end(), Port::compare_by_name);
		for (Port* p: g.second)
		{
			_action_id += 1;
			QAction* action = group_menu->addAction (pixmap_for_port, QString::fromStdString (p->name()), _connect_signal_mapper, SLOT (map()));
			action->setEnabled (!p->connected_to (event_port()));
			_connect_signal_mapper->setMapping (action, _action_id);
			_context_menu_port_map[_action_id] = p;
		}
	}

	// Add port items:
	std::sort (ports.begin(), ports.end(), Port::compare_by_name);
	for (Port* p: ports)
	{
		_action_id += 1;
		QAction* action = unit_menu->addAction (pixmap_for_port, QString::fromStdString (p->name()), _connect_signal_mapper, SLOT (map()));
		action->setEnabled (!p->connected_to (event_port()));
		_connect_signal_mapper->setMapping (action, _action_id);
		_context_menu_port_map[_action_id] = p;
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
		Mutex::Lock lock (*unit_bay()->graph());
		ContextMenuPortMap::iterator a = _context_menu_port_map.find (action_id);
		// FIXME If port is deleted between menu popup and action exec, connect_to() will be executed on singular object.
		if (a != _context_menu_port_map.end())
			a->second->connect_to (event_port());
	}
}


void
Knob::disconnect_port (int action_id)
{
	if (unit_bay())
	{
		Mutex::Lock lock (*unit_bay()->graph());
		ContextMenuPortMap::iterator a = _context_menu_port_map.find (action_id);
		// FIXME If port is deleted between menu popup and action exec, connect_to() will be executed on singular object.
		if (a != _context_menu_port_map.end())
			a->second->disconnect_from (event_port());
	}
}


void
Knob::disconnect_from_all()
{
	if (unit_bay())
	{
		Mutex::Lock lock (*unit_bay()->graph());
		event_port()->disconnect();
	}
}

} // namespace Haruhi

