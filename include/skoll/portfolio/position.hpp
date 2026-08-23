#include "skoll/types.hpp"
#include <cstdint>
namespace skoll::portfolio {
    class Position {
      public:
        void apply_fill(Side side, Price price, Quantity quantity);

        [[nodiscard]] std::int64_t quantity() const;
        [[nodiscard]] std::int64_t cash() const;
        [[nodiscard]] std::int64_t pnl(Price mark_price) const;

      private:
        std::int64_t quantity_{};
        std::int64_t cash_{};
    };
} // namespace skoll::portfolio
