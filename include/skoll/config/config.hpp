#pragma once

#include <string>

#include "skoll/types.hpp"


namespace skoll {
    struct Config {
        std::string ws_url;
        SecurityId security_id;
    };

    Config load_config(const std::string &path);
}
