#pragma once

#include <functional>
#include <string>

#include <ixwebsocket/IXWebSocket.h>

#include "skoll/types.hpp"

namespace skoll::feed {
    class Client {
    public:
        using MessageHandler = std::function<void (const std::string &)>;

        Client( // ctor
            std::string url,
            SecurityId security_id,
            MessageHandler message_handler
        );


        void run();

    private:
        void handle_message(const ix::WebSocketMessagePtr &message);

        void subscribe();

        std::string url_;
        SecurityId security_id_{};
        MessageHandler message_handler_;
        ix::WebSocket web_socket_;
    };
}
