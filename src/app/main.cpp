/***************************************************************************
 *  main.cpp - The tool's main binary
 *
 *  Created:   Mon 19 Apr 16:20:10 CEST 2021
 *  Copyright  2021  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/


#include "app/app.h"

#include <spdlog/spdlog.h>

#include <stdexcept>

int
main(int argc, const char *const argv[])
{
	try {
		tacos::app::Launcher launcher{argc, argv};
		launcher.run();
		return 0;
	} catch (const std::exception &e) {
		SPDLOG_CRITICAL("Exception: {}", e.what());
		return 1;
	}
}
