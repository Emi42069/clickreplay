// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#pragma once

#include "data/Dataset.h"
#include "input/Input.h"
#include "platform/Platform.h"
#include "replay/ReplayConfig.h"
#include "replay/ReplayEngine.h"
#include "recording/Recorder.h"
#include "recording/RecordingConfig.h"
#include "stats/Statistics.h"

#include <string>

enum class Page {
    Dashboard,
    Dataset,
    Recorder,
    Analytics,
    Settings,
    Logs
};

class AppState {
public:
    AppState();
    ~AppState();

    bool load_dataset();

    Dataset dataset;
    Statistics statistics;
    Input input;
    Platform platform;
    ReplayConfig config;
    RecordingConfig recording_config;
    ReplayEngine replay;
    Recorder recorder;

    std::string csv_path;
    Page page = Page::Dashboard;
};
