#pragma once

#include <string>

class Platform {
public:
    bool is_javaw_foreground() const;
    long long send_left_click(std::string& error) const;
};
