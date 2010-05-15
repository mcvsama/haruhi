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

// System:
#include <unistd.h>
#include <signal.h>

// Standard:
#include <cstddef>
#include <iostream>
#include <iterator>
#include <vector>
#include <string>

// Local:
#include <haruhi/utility/backtrace.h>

#include "fail.h"


namespace Haruhi {

void
fail (int signum)
{
	std::vector<const char*> features;
#ifdef HARUHI_SSE3
	features.push_back ("HARUHI_SSE3");
#endif
#ifdef HARUHI_IEEE754
	features.push_back ("HARUHI_IEEE754");
#endif

	std::clog << "Haruhi died by signal. Please submit following bug report to http://haruhi.mulabs.org/report-bug" << std::endl << std::endl;
	std::clog << "Signal: " << signum << std::endl;
	std::clog << "Features: ";
	std::copy (features.begin(), features.end(), std::ostream_iterator<const char*> (std::clog, " "));
	std::clog << std::endl;
	std::clog << "CXXFLAGS: " << CXXFLAGS << std::endl;

	Backtrace::clog();
	// Force coredump if enabled:
	signal (signum, SIG_DFL);
	kill (getpid(), signum);
}

} // namespace Haruhi

