// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#pragma once

#include <string>

class Platform {
public:
    bool is_javaw_foreground() const;
    long long send_left_click(std::string& error) const;
};
