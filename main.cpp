#include <iostream>
#include <optional>
#include <string>

#include "skoll/config/config.hpp"
#include "skoll/feed/capture.hpp"
#include "skoll/feed/client.hpp"
#include "skoll/feed/message.hpp"
#include "skoll/feed/replay.hpp"

#include <ixwebsocket/IXNetSystem.h>

int main() {
    try {
        std::cout << "Sköll v0.1 \n";

        const auto config = skoll::load_config(".env");

        auto message_handler = [](const std::string &message) {
            const auto decoded = skoll::feed::decode_message(message);
            std::cout << "decoded message successfully!!! \n";
        };

        std::optional<skoll::feed::Capture> capture;
        skoll::feed::Client::CaptureHandler capture_handler;

        if (config.capture_path) {
            capture.emplace(*config.capture_path); // construct Capture directly inside the optional
            capture_handler = [&capture](std::string_view payload) { capture->write(payload); };
        }

        if (config.replay_path) {
            skoll::feed::Replay replay(*config.replay_path, message_handler);

            replay.run();
            return 0;
        }

        ix::initNetSystem();
        skoll::feed::Client client(config.ws_url, config.security_id, message_handler,
                                   std::move(capture_handler));

        client.run();

        ix::uninitNetSystem();
    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
