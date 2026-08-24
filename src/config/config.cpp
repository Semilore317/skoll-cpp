#include "skoll/config/config.hpp"

#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace skoll {
    Config load_config(const std::string &path) {
        std::ifstream file(path);

        if (!file.is_open())
            throw std::runtime_error("could not open config file");

        std::unordered_map<std::string, std::string> values;
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#')
                continue;

            const auto separator = line.find('=');

            if (separator == std::string::npos) // ::npos --> not a position in string
                continue;

            const std::string key = line.substr(0, separator);
            const std::string value = line.substr(separator + 1);

            values[key] = value;
        }

        if (!values.contains("WS_URL"))
            throw std::runtime_error("could not find WS_URL in config file");

        if (!values.contains("SECURITY_ID"))
            throw std::runtime_error("could not find SECURITY_ID in config file");

        std::optional<std::string> capture_path;
        if (values.contains("CAPTURE_PATH") && !values.at("CAPTURE_PATH").empty())
            capture_path = values.at("CAPTURE_PATH");

        std::optional<std::string> replay_path;
        if (values.contains("REPLAY_PATH") && !values.at("REPLAY_PATH").empty())
            replay_path = values.at("REPLAY_PATH");

        if (!values.contains("REST_URL"))
            throw std::runtime_error("could not find REST_URL in config file");

        if (!values.contains("USERNAME"))
            throw std::runtime_error("could not find USERNAME in config file");

        if (!values.contains("MAKER_QUANTITY"))
            throw std::runtime_error("could not find MAKER_QUANTITY in config file");

        return Config{
            .ws_url = values.at("WS_URL"),
            .security_id = static_cast<SecurityId>(
                std::stoi(values.at("SECURITY_ID")) // string TO integer
                ),
            .rest_url = values.at("REST_URL"),
            .username = values.at("USERNAME"),
            .maker_quantity = static_cast<Quantity>(
                std::stoul(values.at("MAKER_QUANTITY")) // string to unsigned long
                ),
            .capture_path = capture_path,
            .replay_path = replay_path};
    }
} // namespace skoll
