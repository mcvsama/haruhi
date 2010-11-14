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

// Standards:
#include <iostream>

// System:
#include <signal.h>
#include <locale.h>

// Qt:
#include <QtCore/QTextCodec>

// Haruhi:
#include <haruhi/config/version.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/application/fail.h>
#include <haruhi/utility/backtrace.h>


int main (int argc, char** argv, char** envp)
{
	signal (SIGILL, Haruhi::fail);
	signal (SIGFPE, Haruhi::fail);
	signal (SIGSEGV, Haruhi::fail);

	setenv ("LC_ALL", "POSIX", 1);
	setlocale (LC_ALL, "POSIX");

	// Qt preparations:
	QTextCodec::setCodecForCStrings (QTextCodec::codecForName ("UTF-8"));
	// Now casting QString to std::string|const char* will yield UTF-8 encodec strings.
	// Also std::strings and const chars* are expected to be encoded in UTF-8.

	try {
		if (argc == 2 && (strcmp (argv[1], "-v") == 0 || strcmp (argv[1], "--version") == 0))
			std::cout << "Haruhi  commit: " << Haruhi::Version::commit << "  branch: " << Haruhi::Version::branch << std::endl;
		else
			Haruhi::Haruhi haruhi (argc, argv, envp);
	}
	catch (...)
	{
		Backtrace::clog();
		throw;
	}
}

