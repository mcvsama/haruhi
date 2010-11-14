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

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QDialog>
#include <QtGui/QLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QFileDialog>
#include <QtGui/QTreeWidget>
#include <QtGui/QHeaderView>

// Haruhi:
#include <haruhi/config/all.h>
#include <haruhi/session/session.h>
#include <haruhi/settings/recent_session.h>

// Local:
#include "session_loader.h"


namespace Haruhi {

SessionLoader::SessionLoader (DefaultTab default_tab, RejectButton reject_button, QWidget* parent):
	QDialog (parent),
	_result (NoResult)
{
	setCaption ("Haruhi — Session Control");
	setIcon (QPixmap ("share/images/haruhi.png"));

	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QVBoxLayout* layout = new QVBoxLayout (this, Config::DialogMargin, Config::Spacing);
	layout->setResizeMode (QLayout::Fixed);

	_tabs = new QTabWidget (this);
	_tabs->setTabPosition (QTabWidget::North);
	layout->addWidget (_tabs);

	//
	// New session
	//

	_new_tab = new QWidget (_tabs);
	_tabs->addTab (_new_tab, "&New session");
	QVBoxLayout* new_layout = new QVBoxLayout (_new_tab, 2 * Config::Margin, Config::Spacing);
	QGridLayout* new_grid = new QGridLayout (new_layout, 0, 2, Config::Spacing);
	new_grid->setColSpacing (0, 100);
	new_grid->setColSpacing (1, 150);

	new_grid->addWidget (new QLabel ("Session name:", _new_tab), 0, 0);

	_new_session_name = new QLineEdit ("New session", _new_tab);
	_new_session_name->selectAll();
	_new_session_name->setFocus();
	new_grid->addWidget (_new_session_name, 0, 1);

	new_grid->addWidget (new QLabel ("Audio inputs:", _new_tab), 1, 0);

	_new_session_audio_inputs = new QSpinBox (0, 16, 1, _new_tab);
	_new_session_audio_inputs->setValue (2);
	new_grid->addWidget (_new_session_audio_inputs, 1, 1);

	new_grid->addWidget (new QLabel ("Audio outputs:", _new_tab), 2, 0);

	_new_session_audio_outputs = new QSpinBox (0, 16, 1, _new_tab);
	_new_session_audio_outputs->setValue (2);
	new_grid->addWidget (_new_session_audio_outputs, 2, 1);

	new_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

	//
	// Open session
	//

	_open_tab = new QWidget (_tabs);
	_tabs->addTab (_open_tab, "&Open session");

	QVBoxLayout* open_layout = new QVBoxLayout (_open_tab, 2 * Config::Margin, Config::Spacing);

	open_layout->addItem (new QSpacerItem (0, Config::Spacing, QSizePolicy::Fixed, QSizePolicy::Fixed));
	open_layout->addWidget (new QLabel ("Recent sessions:", _open_tab));
	_recent_listview = new QTreeWidget (_open_tab);
	_recent_listview->header()->hide();
	_recent_listview->setMinimumSize (300, 300);
	_recent_listview->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	_recent_listview->setHeaderLabel ("Session name");
	_recent_listview->setSelectionMode (QTreeWidget::SingleSelection);
	_recent_listview->setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
	_recent_listview->setRootIsDecorated (false);
	QObject::connect (_recent_listview, SIGNAL (itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT (open_recent (QTreeWidgetItem*, int)));
	open_layout->addWidget (_recent_listview);

	QHBoxLayout* open_file_layout = new QHBoxLayout (open_layout, Config::Spacing);

	open_file_layout->addWidget (new QLabel ("Load from file:", _open_tab));
	open_file_layout->addItem (new QSpacerItem (Config::Spacing, 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
	QPushButton* browse_button = new QPushButton ("Browse", _open_tab);
	browse_button->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
	QObject::connect (browse_button, SIGNAL (clicked()), this, SLOT (browse_file()));
	open_file_layout->addWidget (browse_button);
	open_file_layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

	// Populate recent_listview:
	for (Settings::RecentSessions::iterator rs = Settings::recent_sessions().begin(); rs != Settings::recent_sessions().end(); ++rs)
		new RecentSessionItem (_recent_listview, *rs);

	if (_recent_listview->invisibleRootItem()->childCount() > 0)
		_recent_listview->setCurrentItem (_recent_listview->invisibleRootItem()->child (0));

	//
	// Buttons
	//

	QHBoxLayout* buttons_layout = new QHBoxLayout (layout, Config::Spacing);
	buttons_layout->addItem (new QSpacerItem (0, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	_open_button = new QPushButton (Config::Icons16::new_(), "Open", this);
	_open_button->setDefault (true);
	QObject::connect (_open_button, SIGNAL (clicked()), this, SLOT (validate_and_accept()));
	buttons_layout->addWidget (_open_button);

	_quit_button = new QPushButton (Config::Icons16::exit(), reject_button == CancelButton ? "Cancel" : "Quit", this);
	QObject::connect (_quit_button, SIGNAL (clicked()), this, SLOT (reject()));
	buttons_layout->addWidget (_quit_button);

	QObject::connect (_tabs, SIGNAL (currentChanged (QWidget*)), this, SLOT (update_widgets()));

	switch (default_tab)
	{
		case NewTab:	_tabs->showPage (_new_tab); break;
		case OpenTab:	_tabs->showPage (_open_tab); break;
	}

	update_widgets();
}


void
SessionLoader::apply (Session* session)
{
	switch (_result)
	{
		case NoResult:
			break;

		case NewSession:
			session->set_name (_new_session_name->text());
			// Create audio inputs/outputs:
			for (int i = 0, n = _new_session_audio_inputs->value(); i < n; ++i)
				session->audio_backend()->create_input (QString ("in %1").arg (i + 1));
			for (int i = 0, n = _new_session_audio_outputs->value(); i < n; ++i)
				session->audio_backend()->create_output (QString ("out %1").arg (i + 1));
			break;

		case OpenSession:
			session->load_session (_file_name);
			break;
	}
}


void
SessionLoader::update_widgets()
{
	_open_button->setText (_tabs->currentPage() == _new_tab ? "New" : "Open");
}


void
SessionLoader::validate_and_accept()
{
	if (_tabs->currentPage() == _new_tab)
	{
		_result = NewSession;
		accept();
	}
	else if (_tabs->currentPage() == _open_tab)
	{
		_result = OpenSession;
		if (!_recent_listview->selectedItems().empty())
			open_recent (_recent_listview->selectedItems().front(), 0);
	}
}


void
SessionLoader::browse_file()
{
	QFileDialog* file_dialog = new QFileDialog (this, "Load session", ".", QString());
	file_dialog->setMode (QFileDialog::ExistingFile);
	file_dialog->setNameFilter ("All Haruhi session files (*.haruhi-session)");
	if (file_dialog->exec() == QFileDialog::Accepted)
	{
		_result = OpenSession;
		_file_name = file_dialog->selectedFile();
		accept();
	}
}


void
SessionLoader::open_recent (QTreeWidgetItem* item, int)
{
	RecentSessionItem* recent_session_item = dynamic_cast<RecentSessionItem*> (item);
	if (recent_session_item)
	{
		_result = OpenSession;
		_file_name = recent_session_item->recent_session.file_name;
		accept();
	}
}

} // namespace Haruhi

