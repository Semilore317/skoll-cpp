#include <doctest/doctest.h>

#include "skoll/book/replica.hpp"
#include "skoll/feed/message.hpp"
#include "skoll/strategy/maker.hpp"

TEST_CASE("maker quotes best bid and ask") {
    skoll::book::Replica replica(1);

    skoll::feed::BookMessage message;
    message.security_id = 1;
    message.bid = 100;
    message.ask = 105;
    message.spread = 5;
    message.bids = {{100, 10}};
    message.asks = {{105, 10}};

    replica.apply(message);

    skoll::strategy::Maker maker(2);

    const auto orders = maker.quote(replica);

    REQUIRE(orders.size() == 2);

    CHECK(orders[0].side == skoll::Side::buy);
    CHECK(orders[0].price == 100);
    CHECK(orders[0].quantity == 2);

    CHECK(orders[1].side == skoll::Side::sell);
    CHECK(orders[1].price == 105);
    CHECK(orders[1].quantity == 2);
}

TEST_CASE("maker does nothing without a complete market") {
    skoll::book::Replica replica(1);
    skoll::strategy::Maker maker(1);

    CHECK(maker.quote(replica).empty());
}

TEST_CASE("maker rejects zero quantity") {
    CHECK_THROWS_AS(
        skoll::strategy::Maker(0),
        std::invalid_argument);
}