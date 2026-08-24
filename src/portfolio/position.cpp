#include "skoll/portfolio/position.hpp"

namespace skoll::portfolio {
    void Position::apply_fill(
        const Side side,
        const Price price,
        const Quantity quantity) {
        const auto signed_quantity = static_cast<std::int64_t>(quantity);
        const auto value =
            static_cast<std::int64_t>(price) * signed_quantity;

        if (side == Side::buy) {
            quantity_ += signed_quantity;
            cash_ -= value;
        } else {
            quantity_ -= signed_quantity;
            cash_ += value;
        }
    }
    std::int64_t Position::quantity() const {
        return quantity_;
    }

    std::int64_t Position::cash() const {
        return cash_;
    }

    std::int64_t Position::pnl(const Price mark_price) const {
        return cash_ + quantity_ * static_cast<std::int64_t>(mark_price);
    }
} // namespace skoll::portfolio
