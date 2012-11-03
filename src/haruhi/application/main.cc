/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
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

// Haruhi:
#include <haruhi/config/version.h>
#include <haruhi/application/haruhi.h>
#include <haruhi/application/services.h>
#include <haruhi/application/fail.h>
#include <haruhi/utility/backtrace.h>
#include <haruhi/utility/fast_pow.h>


int main (int argc, char** argv, char** envp)
{
	signal (SIGILL, Haruhi::fail);
	signal (SIGFPE, Haruhi::fail);
	signal (SIGSEGV, Haruhi::fail);

	setenv ("LC_ALL", "POSIX", 1);
	setlocale (LC_ALL, "POSIX");

	// Lib initializations:
	LookupPow::initialize();
#ifdef HARUHI_HAS_SSE_POW
	SSEPow::initialize();
#endif

	try {
		if (argc == 2 && (strcmp (argv[1], "-v") == 0 || strcmp (argv[1], "--version") == 0))
		{
			std::cout << "Haruhi" << std::endl;
			std::cout << "Commit: " << Haruhi::Version::commit << std::endl;
			std::cout << "Branch: " << Haruhi::Version::branch << std::endl;
			std::clog << "Features: ";
			std::vector<const char*> features = Haruhi::Services::features();
			std::copy (features.begin(), features.end(), std::ostream_iterator<const char*> (std::clog, " "));
			std::clog << std::endl;
		}
		else
			Haruhi::Haruhi haruhi (argc, argv, envp);
	}
	catch (...)
	{
		Backtrace::clog();
		throw;
	}

	// Deinit:
	LookupPow::deinitialize();
#ifdef HARUHI_HAS_SSE_POW
	SSEPow::deinitialize();
#endif

	return EXIT_SUCCESS;
}

