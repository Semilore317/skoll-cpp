#include "skoll/book/replica.hpp"
#include "skoll/feed/message.hpp"
#include "skoll/types.hpp"

#include <optional>
#include <stdexcept>
#include <vector>

namespace skoll::book {
    Replica::Replica(const SecurityId security_id) : security_id_(security_id) {}

    void Replica::apply(const feed::BookMessage &message) {
        if (message.security_id != security_id_)
            throw std::invalid_argument("book message security ID does not match replica");

        best_bid_ = message.bid;
        best_ask_ = message.ask;
        spread_ = message.spread;

        // replaces existing levels with new ones since valkyrie sends full book snapshots
        bids_ = message.bids;
        asks_ = message.asks;
    }

    SecurityId Replica::security_id() const noexcept {
        return security_id_;
    }

    std::optional<Price> Replica::best_bid() const noexcept {
        return best_bid_;
    }

    std::optional<Price> Replica::best_ask() const noexcept {
        return best_ask_;
    }

    std::optional<Price> Replica::spread() const noexcept {
        return spread_;
    }

    const std::vector<feed::Level> &Replica::bids() const noexcept {
        return bids_;
    }

    const std::vector<feed::Level> &Replica::asks() const noexcept {
        return asks_;
    }
} // namespace skoll::book
