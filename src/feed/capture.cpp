#include <fstream>
#include <ios>
#include <skoll/feed/capture.hpp>
#include <stdexcept>
#include <string_view>

namespace skoll::feed {

    Capture::Capture(const std::string &path)
        : file_(path, std::ios::app) { // ios::app to append to the file
        if (!file_)
            throw std::runtime_error("Failed to open capture file: " + path);
    }

    void Capture::write(std::string_view payload) {
        file_ << payload << '\n';

        if (!file_.is_open())
            throw std::runtime_error("Failed to write to the capture file");
    }
} // namespace skoll::feed
