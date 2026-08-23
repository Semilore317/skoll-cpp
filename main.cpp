#include <iostream>
#include <string>

#include "skoll/config/config.hpp"
#include "skoll/feed/client.hpp"

#include <ixwebsocket/IXNetSystem.h>

int main() {
    try {
        std::cout << "Sköll v0.1 \n";

        ix::initNetSystem();

        const auto [ws_url, security_id] = skoll::load_config(".env");

        skoll::feed::Client client(
            ws_url,
            security_id,
            [](const std::string &message) {
                std::cout << message << "\n";
            }
        );

        client.run();

        ix::uninitNetSystem();
    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
