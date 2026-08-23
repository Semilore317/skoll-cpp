#pragma once

#include <functional>
#include <ixwebsocket/IXWebSocket.h>
#include <string>
#include <string_view>

#include "skoll/types.hpp"

namespace skoll::feed {
    class Client {
      public:
        using MessageHandler = std::function<void(const std::string &)>;
        using CaptureHandler = std::function<void(std::string_view)>;

        Client( // ctor
            std::string url, SecurityId security_id, MessageHandler message_handler,
            CaptureHandler capture_handler = {});

        void run();

      private:
        void handle_message(const ix::WebSocketMessagePtr &message);

        void subscribe();

        std::string url_;
        SecurityId security_id_{};
        MessageHandler message_handler_;
        CaptureHandler capture_handler_;
        ix::WebSocket web_socket_;
    };
} // namespace skoll::feed
