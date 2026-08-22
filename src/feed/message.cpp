#include "skoll/feed/message.hpp"

#include <nlohmann/json.hpp>

#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <vector>
#include <string_view>

namespace skoll::feed {
    namespace { // private hack for non-class files
        using Json = nlohmann::json;

        Side parse_side(const Json &value) {
            const auto side = value.get<std::string>();

            if (side == "buy")
                return Side::buy;

            if (side == "sell")
                return Side::sell;

            throw DecodeError("aggressor side must be `buy` or `sell`");
        }

        Quantity parse_quantity(const Json &value) {
            if (!value.is_number_unsigned()) {
                throw DecodeError("quantity must be a non-negative integer");
            }

            const auto quantity = value.get<std::uint64_t>();

            if (quantity > std::numeric_limits<Quantity>::max()) {
                throw DecodeError("quantity exceeds Uint32");
            }

            return static_cast<Quantity>(quantity);
        }

        LevelQuantity parse_level_quantity(const Json &value) {
            if (!value.is_number_unsigned()) {
                throw DecodeError("level quantity must be a non-negative integer");
            }

            const auto quantity = value.get<std::uint64_t>();
            if (quantity > static_cast<std::uint64_t>(std::numeric_limits<LevelQuantity>::max())) {
                throw DecodeError("level quantity exceeds Int64");
            }

            return static_cast<LevelQuantity>(quantity);
        }

        std::optional<Price> parse_optional_price(const Json &value) {
            if (value.is_null())
                return std::nullopt;

            return value.get<Price>();
        }

        std::vector<Level> parse_levels(const Json &values) {
            if (!values.is_array())
                throw DecodeError("book levels must be an array");

            std::vector<Level> levels;
            levels.reserve(values.size()); // reserve allocates more space while the vector is still empty

            for (const auto &value: values) {
                levels.push_back(
                    Level{
                        .price = value.at("price").get<Price>(),
                        .quantity = parse_level_quantity(value.at("quantity"))
                    });
            }

            return levels;
        }

        ObservedPrice parse_observed_price(const Json &value) {
            if (!value.is_number()) {
                throw DecodeError("Trade price must be numeric");
            }

            /*
             * valkyrie sends markettrade prices in  cents with two decimal digits
             * 243444 --> 2434.44c --> $24.3444
             */

            // floating points exist only after crossing the JSON boundary
            const auto scaled = value.get<long double>() * 100.0L;

            const auto rounded = std::round(scaled);

            if (!std::isfinite(scaled) || std::fabs(scaled - rounded) > 0.000001L) {
                throw DecodeError("Trade price has more than two digits for cents");
            }

            return ObservedPrice{
                static_cast<std::int64_t>(rounded)
            };
        }

        BookMessage parse_book(const Json &value) {
            return BookMessage{
                .security_id = value.at("securityId").get<SecurityId>(),

                .bid = parse_optional_price(value.at("bid")),
                .ask = parse_optional_price(value.at("ask")),
                .spread = parse_optional_price(value.at("spread")),

                .bids = parse_levels(value.at("bids")),
                .asks = parse_levels(value.at("asks")),
            };
        }

        TradeMessage parse_trade(const Json& value) {
            return TradeMessage{
                .security_id =
                    value.at("securityId").get<SecurityId>(),

                .bid_order_id =
                    value.at("bidOrderId").get<OrderId>(),

                .ask_order_id =
                    value.at("askOrderId").get<OrderId>(),

                .price =
                    value.at("price").get<Price>(),

                .quantity =
                    parse_quantity(value.at("quantity")),

                .filled_at =
                    value.at("filledAt").get<std::string>(),

                .aggressor_side =
                    parse_side(value.at("aggressorSide"))
            };
        }

        MarketTradeMessage parse_market_trade(
            const Json& value
        ) {
            return MarketTradeMessage{
                .security_id =
                    value.at("securityId").get<SecurityId>(),

                .price =
                    parse_observed_price(value.at("price")),

                .quantity =
                    parse_quantity(value.at("quantity")),

                .occurred_at =
                    value.at("occurredAt").get<std::string>(),

                .aggressor_side =
                    parse_side(value.at("aggressorSide"))
            };
        }
    }

    Message decode_message(std::string_view raw_message) {
        try {
            const auto value = Json::parse(
                raw_message.begin(),
                raw_message.end()
            );

            const auto type = value.at("type").get<std::string>();

            if (type == "book")
                return parse_book(value);

            if (type == "trade")
                return parse_trade(value);

            if (type == "marketTrade")
                return parse_market_trade(value);

            throw DecodeError(
                "unsupported message type `" + type + "`"
            );
        }catch (const DecodeError&) {
            throw; // preserve errors deliberately
        }catch (const Json::exception& error) {
            // convert nlohmann::Json errors into public decoder-specific exception
            throw DecodeError(
                "Invalid market-data JSON" + std::string{error.what()}
            );
        }
    }
}
