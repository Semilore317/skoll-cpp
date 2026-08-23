#pragma once

#include <fstream>
#include <string>
#include <string_view>

namespace skoll::feed {
    class Capture {
      public:
        explicit Capture(const std::string &path); // explicit prevents implicit conversion from
                                                   // std::string to Capture;
        void write(std::string_view payload);      // string_view means write() can view the string
                                                   // without copying it;

      private:
        std::ofstream file_;
    };
} // namespace skoll::feed
