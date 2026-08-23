#include "skoll/book/replica.hpp"
#include "skoll/feed/message.hpp"

#include <doctest/doctest.h>
#include <stdexcept>

TEST_CASE("replica starts empty") {
    skoll::book::Replica replica(1);

    CHECK(replica.security_id() == 1);
    CHECK_FALSE(replica.best_bid().has_value());
    CHECK_FALSE(replica.best_ask().has_value());
    CHECK_FALSE(replica.spread().has_value());

    CHECK(replica.bids().empty());
    CHECK(replica.asks().empty());
}

TEST_CASE("replica applies book snapshot") {
    skoll::book::Replica replica(1);

    skoll::feed::BookMessage message;

    message.security_id = 1;
    message.bid = 100;
    message.ask = 105;
    message.spread = 5;

    message.bids = { {100, 10}, {99, 20}};
    message.asks = { {105, 15}, {106, 25} };

    replica.apply(message);

    REQUIRE(replica.best_bid().has_value());
    REQUIRE(replica.best_ask().has_value());
    REQUIRE(replica.spread().has_value());

    CHECK(*replica.best_bid() == 100); // replica.best_bid().value()  = 100
    CHECK(*replica.best_ask() == 105);
    CHECK(*replica.spread() == 5);

    REQUIRE(replica.bids().size() == 2);
    REQUIRE(replica.asks().size() == 2);

    CHECK(replica.bids()[0].price == 100);
    CHECK(replica.bids()[0].quantity == 10);
    CHECK(replica.bids()[1].price == 99);
    CHECK(replica.bids()[1].quantity == 20);

    CHECK(replica.asks()[0].price == 105);
    CHECK(replica.asks()[0].quantity == 15);
    CHECK(replica.asks()[1].price == 106);
    CHECK(replica.asks()[1].quantity == 25);
}

TEST_CASE("replica replaces previous snapshot"){
    skoll::book::Replica replica(1);

    skoll::feed::BookMessage first;
    first.security_id = 1;
    first.bid = 100;
    first.ask = 105;
    first.spread = 5;
    first.bids = {{100,10}, {99,20}};
    first.asks = {{105, 15}, {106, 25}};

    replica.apply(first);

    skoll::feed::BookMessage second;
    second.security_id = 1;
    second.bid = 101;
    second.ask = 103;
    second.spread = 2;
    second.bids = {{101,30}};
    second.asks = {{103, 40}};

    replica.apply(second);
    
    CHECK(*replica.best_bid() == 101);
    CHECK(*replica.best_ask() == 103);
    CHECK(*replica.spread() == 2);

    REQUIRE(replica.bids().size() == 1);
    REQUIRE(replica.asks().size() == 1);

    CHECK(replica.bids()[0].price == 101);
    CHECK(replica.bids()[0].quantity == 30);

    CHECK(replica.asks()[0].price == 103);
    CHECK(replica.asks()[0].quantity == 40);
}

TEST_CASE("replica rejects book for different security"){
    skoll::book::Replica replica(1);

    skoll::feed::BookMessage message;
    message.security_id = 2;

    CHECK_THROWS_AS(
        replica.apply(message),
        std::invalid_argument
    );
}