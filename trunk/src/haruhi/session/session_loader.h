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

#ifndef HARUHI__SESSION__SESSION_LOADER_H__INCLUDED
#define HARUHI__SESSION__SESSION_LOADER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QWidget>
#include <QtGui/QDialog>
#include <QtGui/QTabWidget>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

// Haruhi:
#include <haruhi/settings/session_loader_settings.h>


namespace Haruhi {

class Session;


class SessionLoader: public QDialog
{
	Q_OBJECT

	class RecentSessionItem: public QTreeWidgetItem
	{
	  public:
		RecentSessionItem (QTreeWidget* parent, SessionLoaderSettings::RecentSession const& recent_session):
			QTreeWidgetItem (parent, QStringList (recent_session.name + QString (" (%1)").arg (::basename (recent_session.file_name.toUtf8())))),
			recent_session (recent_session)
		{
			setup();
		}

		void
		setup (void)
		{
			QSize s = sizeHint (0);
			if (s.height() < 18)
			{
				s.setHeight (18);
				setSizeHint (0, s);
			}
		}

	  public:
		SessionLoaderSettings::RecentSession recent_session;
	};

  public:
	enum RejectButton { CancelButton, QuitButton };
	enum DefaultTab { NewTab, OpenTab };
	enum Result { NoResult, NewSession, OpenSession };

  public:
	SessionLoader (DefaultTab, RejectButton, QWidget* parent);

	void
	apply (Session*);

  private slots:
	void
	update_widgets();

	void
	validate_and_accept();

	void
	browse_file();

	void
	open_recent (QTreeWidgetItem*, int);

  private:
	QTabWidget*		_tabs;
	QWidget*		_new_tab;
	QWidget*		_open_tab;
	QPushButton*	_open_button;
	QPushButton*	_quit_button;
	QLineEdit*		_new_session_name;
	QSpinBox*		_new_session_audio_inputs;
	QSpinBox*		_new_session_audio_outputs;
	QTreeWidget*	_recent_listview;
	Result			_result;
	QString			_file_name;
};

} // namespace Haruhi

#endif

