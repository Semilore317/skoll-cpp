#include <doctest/doctest.h>

#include <skoll/portfolio/position.hpp>

TEST_CASE("position starts flat") {
    skoll::portfolio::Position position;

    CHECK(position.quantity() == 0);
    CHECK(position.cash() == 0);
    CHECK(position.pnl(100) == 0);
}

TEST_CASE("buy fill updates position and cash") {
    skoll::portfolio::Position position;

    position.apply_fill(skoll::Side::buy, 100, 5);

    CHECK(position.quantity() == 5);
    CHECK(position.cash() == -500);
    CHECK(position.pnl(100) == 0);
    CHECK(position.pnl(105) == 25);
}

TEST_CASE("sell fill updates position and cash") {
    skoll::portfolio::Position position;

    position.apply_fill(skoll::Side::sell, 100, 5);

    CHECK(position.quantity() == -5);
    CHECK(position.cash() == 500);
    CHECK(position.pnl(95) == 25);
}

TEST_CASE("round trip realizes profit") {
    skoll::portfolio::Position position;

    position.apply_fill(skoll::Side::buy, 100, 10);
    position.apply_fill(skoll::Side::sell, 105, 10);

    CHECK(position.quantity() == 0);
    CHECK(position.cash() == 50);
    CHECK(position.pnl(100) == 50);
}