#pragma once

#include <optional>
#include <string>

#include "skoll/types.hpp"

namespace skoll {
    struct Config {
        std::string ws_url;
        SecurityId security_id;

        std::string rest_url;
        std::string username;
        Quantity maker_quantity;

        std::optional<std::string> capture_path;
        std::optional<std::string> replay_path;
    };

    Config load_config(const std::string &path);
} // namespace skoll
