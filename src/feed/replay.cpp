#include "skoll/feed/replay.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

namespace skoll::feed {
    Replay::Replay(std::string path, MessageHandler message_handler)
        : path_(std::move(path)), message_handler_(message_handler) {}

    void Replay::run() {
        std::ifstream file(path_);

        if (!file)
            throw std::runtime_error("failed to open replay file: " + path_);

        std::string line;

        while (std::getline(file, line)) {
            if (!line.empty())
                message_handler_(line);
        }

        if (!file.eof())
            throw std::runtime_error("failed while reading replay file: " + path_);
    }
} // namespace skoll::feed
