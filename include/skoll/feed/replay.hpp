#pragma once

#include <functional>
#include <string>

namespace skoll::feed {
    class Replay {
      public:
        using MessageHandler = std::function<void(const std::string &)>;

        Replay(std::string path, MessageHandler message_handler);
        void run();

      private:
        std::string path_;
        MessageHandler message_handler_;
    };
} 
