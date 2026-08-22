#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include <string_view>

#include "../types.hpp"

namespace skoll::feed {
    struct Level {
        Price price{};
        LevelQuantity quantity{};
    };

    struct BookMessage {
        SecurityId security_id{};

        std::optional<Price> bid;
        std::optional<Price> ask;
        std::optional<Price> spread;

        std::vector<Level> bids;
        std::vector<Level> asks;
    };

    struct TradeMessage {
        SecurityId security_id{};

        OrderId bid_order_id{};
        OrderId ask_order_id{};

        Price price{};
        Quantity quantity{};

        std::string filled_at{};
        Side aggressor_side{};
    };

    struct MarketTradeMessage {
        SecurityId security_id{};

        ObservedPrice price{};
        Quantity quantity{};

        std::string occurred_at{};
        Side aggressor_side{};
    };


    // since it tracks only one of each at any point in time
    using Message = std::variant<
        BookMessage,
        TradeMessage,
        MarketTradeMessage
    >;


    class DecodeError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    [[nodiscard]] // tells the compiler to warn if i ignore it
    Message decode_message(std::string_view raw_message);
}
