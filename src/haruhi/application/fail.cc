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
#include <haruhi/config/version.h>
#include <haruhi/utility/backtrace.h>

#include "fail.h"
#include "haruhi.h"


namespace Haruhi {

void
fail (int signum)
{
	std::vector<const char*> features = ::Haruhi::Haruhi::features();
	std::clog << "------------------------------------------------------------------------------------------------" << std::endl;
	std::clog << "Haruhi died by signal. Please submit following bug report to http://haruhi.mulabs.org/report-bug" << std::endl << std::endl;
	std::clog << "  signal: " << signum << std::endl;
	std::clog << "  source info: " << std::endl;
	std::cout << "    commit: " << ::Haruhi::Version::commit << std::endl;
	std::cout << "    branch: " << ::Haruhi::Version::branch << std::endl;
	std::clog << "  features: ";
	std::copy (features.begin(), features.end(), std::ostream_iterator<const char*> (std::clog, " "));
	std::clog << std::endl;
	std::clog << "  backtrace:" << std::endl;
	Backtrace::clog();
	std::clog << "  CXXFLAGS: " << CXXFLAGS << std::endl << std::endl;
	// Force coredump if enabled:
	signal (signum, SIG_DFL);
	kill (getpid(), signum);
}

} // namespace Haruhi

