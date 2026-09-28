// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#include "AppState.h"

#include <iomanip>
#include <random>
#include <sstream>

AppState::AppState()
    : replay(dataset, input, platform, statistics) {
}

AppState::~AppState() {
    replay.stop();
}

bool AppState::load_dataset() {
    if (csv_path.empty()) {
        statistics.log("Select a CSV file first.");
        return false;
    }

    std::string error;
    if (!dataset.load_csv(csv_path, error)) {
        statistics.log(error);
        return false;
    }

    if (config.random_seed) {
        std::random_device random_device;
        config.seed = static_cast<std::uint32_t>(random_device());
    }

    dataset.shuffle(config.seed);
    statistics.reset();

    std::ostringstream message;
    message << "Loaded " << dataset.size() << " intervals. Seed=" << config.seed;
    statistics.log(message.str());

    std::ostringstream cps;
    cps << std::fixed << std::setprecision(2)
        << "Theoretical dataset CPS: " << dataset.cps();
    statistics.log(cps.str());

    return true;
}
