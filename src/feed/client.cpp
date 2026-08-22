#include "skoll/feed/client.hpp"

#include <iostream>
#include  <nlohmann/json.hpp>
#include <stdexcept>
#include <utility>

namespace skoll::feed {
    using Json = nlohmann::json;

    Client::Client(
        std::string url,
        SecurityId security_id,
        MessageHandler message_handler
    ) : url_(std::move(url)),
        security_id_(security_id),
        message_handler_(std::move(message_handler)) //ctor
    {
        web_socket_.setUrl(url_);

        web_socket_.setOnMessageCallback(
            // when ixwebsocket gives a message,
            // call THIS client object's handle_message function to handle it
            // c++ lambdas are ugly
            /*
             * [capture](parameters){
             *  body
             * }
             */
            [this](const ix::WebSocketMessagePtr &message) {
                handle_message(message);
            }
        );
    }

    void Client::run() {
        // connection timeout = 10 seconds
        const auto connection = web_socket_.connect(10);

        if (!connection.success)
            throw std::runtime_error("failed to connect to websocket \n" + connection.errorStr);

        web_socket_.run();
    }

    void Client::handle_message(const ix::WebSocketMessagePtr &message) {
        if (message->type == ix::WebSocketMessageType::Open) {
            std::cout << "connected to market data feed \n";
            subscribe();
            return;
        }

        if (message->type == ix::WebSocketMessageType::Message) {
            message_handler_(message->str);
            return;
        }

        if (message->type == ix::WebSocketMessageType::Error)
            std::cerr
                    << "market data error: "
                    << message->errorInfo.reason
                    << '\n';
    }

    void Client::subscribe() {
        const Json request = {
            {"type", "subscribe"},
            {"securityId", security_id_}
        };

        const auto result = web_socket_.send(request.dump());

        if (!result.success)
            std::cerr << "failed to send subscription \n";
    }
}
