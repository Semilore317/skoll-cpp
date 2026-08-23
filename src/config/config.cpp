#include "skoll/config/config.hpp"

#include <fstream>
#include <unordered_map>
#include <stdexcept>

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

            const auto separator = line.find("=");

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

        return Config{
            .ws_url = values.at("WS_URL"),
            .security_id = static_cast<SecurityId>(
                std::stoi(values.at("SECURITY_ID")) //string TO integer
            ),
        };
    }
}
