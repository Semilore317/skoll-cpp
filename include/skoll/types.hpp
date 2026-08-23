#pragma once

#include <cstdint>

namespace skoll {
    using Price = std::int64_t;
    using Quantity = std::uint32_t;
    using LevelQuantity = std::int64_t;
    using SecurityId = std::int64_t;
    using OrderId = std::int64_t;

    enum class Side {
        buy,
        sell
    };

    // since LOBSTER reports prices with 4 decimal places for cents
    // 243444 --> 2434.44c --> $24.3444
    struct ObservedPrice {
        std::int64_t hundredths_of_a_cent{};
    };
}
