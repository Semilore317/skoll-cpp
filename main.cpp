#include <algorithm>
#include <iostream>
#include <optional>
#include <string>

#include "skoll/config/config.hpp"
#include "skoll/feed/capture.hpp"
#include "skoll/feed/client.hpp"
#include "skoll/feed/message.hpp"

#include <ixwebsocket/IXNetSystem.h>

struct Config {
    std::string ws_url;
    skoll::SecurityId security_id;
    std::optional<std::string> capture_path;
};

int main() {
    try {
        std::cout << "Sköll v0.1 \n";

        ix::initNetSystem();

        const auto config = skoll::load_config(".env");

        std::optional<skoll::feed::Capture> capture;
        skoll::feed::Client::CaptureHandler capture_handler;

        if (config.capture_path) {
            capture.emplace(*config.capture_path);  // construct Capture directly inside the optional
            capture_handler = [&capture](std::string_view payload) {
                capture->write(payload);
            };
        }

        skoll::feed::Client client(
            config.ws_url, 
            config.security_id,
            [](const std::string &message) {
                const auto decoded = skoll::feed::decode_message(message);
                std::cout << "decoded message successfully!!!" << "\n";
            },
            std::move(capture_handler)
            );

        client.run();

        ix::uninitNetSystem();
    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
