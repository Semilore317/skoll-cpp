#pragma once

#include "skoll/feed/message.hpp"
#include "skoll/types.hpp"

#include <optional>
#include <vector>

namespace skoll::book {
    class Replica {
      public:
        explicit Replica(SecurityId security_id);

        void apply(const feed::BookMessage &message);

        // nodiscard tells the compiler to warn if the return value is not used
        // noexcept means the function does not throw exceptions
        // const means the function does not modify the object
        [[nodiscard]] SecurityId security_id() const noexcept;
        [[nodiscard]] std::optional<Price> best_bid() const noexcept;
        [[nodiscard]] std::optional<Price> best_ask() const noexcept;
        [[nodiscard]] std::optional<Price> spread() const noexcept;

        [[nodiscard]] const std::vector<feed::Level> &bids() const noexcept;
        [[nodiscard]] const std::vector<feed::Level> &asks() const noexcept;

      private:
        SecurityId security_id_;

        std::optional<Price> best_bid_;
        std::optional<Price> best_ask_;

        std::optional<Price> spread_;
        
        std::vector<feed::Level> bids_;
        std::vector<feed::Level> asks_;
        
    };
} // namespace skoll::book
