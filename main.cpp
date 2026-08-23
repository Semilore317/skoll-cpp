#include <iostream>
#include <string>

#include "skoll/config/config.hpp"
#include "skoll/feed/client.hpp"


int main() {
    std::cout << "Sköll v0.1" << std::endl;

    const auto [ws_url, security_id] = skoll::load_config(".env");

    skoll::feed::Client client(
        ws_url,
        security_id,
        [](const std::string &message) {
            std::cout << message << "\n";
        }
    );

    client.run();
}
