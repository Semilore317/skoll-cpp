#include "skoll/strategy/maker.hpp"
#include "skoll/types.hpp"

namespace skoll::strategy {
    Maker::Maker(Quantity quantity) : quantity_(quantity) {
        if (quantity_ <= 0)
            throw std::invalid_argument("quantity must be positive");
    }

    std::vector<OrderIntent> Maker::quote(const book::Replica &book) const {
        const auto bid = book.best_bid();
        const auto ask = book.best_ask();

        if (!bid.has_value() || !ask.has_value())
            return {};

        if (*bid >= *ask)
            return {};

        return {{Side::buy, *bid, quantity_}, {Side::sell, *ask, quantity_}};
    }
} // namespace skoll::strategy
