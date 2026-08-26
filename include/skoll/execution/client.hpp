#pragma once

#include "skoll/types.hpp"
#include <stdexcept>
#include <string>
#include <vector>

namespace skoll::execution {
    struct Fill {
        OrderId bid_order_id{};
        OrderId ask_order_id{};
        Price price{};
        Quantity quantity{};
    };
    struct OrderAck {

        OrderId order_id{};
        bool matched{};
        std::vector<Fill> fills;
    };

    class ExecutionError : public std::runtime_error {
      public:
        using std::runtime_error::runtime_error;
    };

    class Client {
      public:
        Client(std::string base_url, std::string username);

        [[nodiscard]]
        OrderAck place_order(
            SecurityId security_id,
            Side side,
            Price price,
            Quantity quantity) const;

        void cancel_order(
            SecurityId security_id,
            OrderId order_id) const;

      private:
        std::string base_url_;
        std::string username_;
    };
} // namespace skoll::execution
