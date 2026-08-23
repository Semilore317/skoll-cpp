#pragma once

#include "skoll/book/replica.hpp"
#include "skoll/types.hpp"

#include <vector>

namespace skoll::strategy {
    struct OrderIntent {
        Side side{};
        Price price{};
        Quantity quantity{};
    };

    class Maker {
      public:
        explicit Maker(Quantity quantity);

        [[nodiscard]]
        std::vector<OrderIntent> quote(const book::Replica &book) const;

      private:
        Quantity quantity_;
    };
} // namespace skoll::strategy
