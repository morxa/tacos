/***************************************************************************
 *  main.cpp - The tool's main app binary of the GOLOG variety
 *
 *  Created:   Tue 20 Nov 18:40:33 CEST 2022
 *  Copyright  2022  Daniel Swoboda <swoboda@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "golog_app/app.h"

#include <spdlog/spdlog.h>

#include <stdexcept>

int
main(int argc, const char *const argv[])
{
	try {
		tacos::golog_app::Launcher launcher{argc, argv};
		launcher.run();
		return 0;
	} catch (const std::exception &e) {
		SPDLOG_CRITICAL("Exception: {}", e.what());
		return 1;
	}
}
