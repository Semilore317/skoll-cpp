#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

#include <ixwebsocket/IXNetSystem.h>

#include "skoll/book/replica.hpp"
#include "skoll/config/config.hpp"
#include "skoll/execution/client.hpp"
#include "skoll/feed/capture.hpp"
#include "skoll/feed/client.hpp"
#include "skoll/feed/message.hpp"
#include "skoll/feed/replay.hpp"
#include "skoll/portfolio/position.hpp"
#include "skoll/strategy/maker.hpp"
#include "skoll/types.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    try {
        std::cout << "Sköll v0.1\n";

        const auto config = skoll::load_config(".env");

        skoll::book::Replica book(config.security_id);
        skoll::strategy::Maker maker(config.maker_quantity);
        skoll::portfolio::Position position;

        std::optional<skoll::execution::Client> execution;

        if (!config.replay_path) {
            execution.emplace(config.rest_url, config.username);
        }

        // represensts the currently resting quote for a single side
        // we need the order id so it can be cancelled when the quote changes
        struct ActiveQuote {
            skoll::OrderId order_id{};
            skoll::Price price{};
        };

        // this lets us correctly handle partial fills.
        // it tracks the remaining quantity on each of Sköll's orders
        std::unordered_map<skoll::OrderId, skoll::Quantity> own_orders;

        /*
        std::optional<skoll::Price> last_bid;
        std::optional<skoll::Price> last_ask;
        */

        std::optional<ActiveQuote> active_bid;
        std::optional<ActiveQuote> active_ask;

        // replay does not submit real orders, so prices alone are enough
        // to avoid repeatedly printing the same quote.
        std::optional<skoll::Price> last_replay_bid;
        std::optional<skoll::Price> last_replay_ask;

        auto print_position = [&]() {
            const auto bid = book.best_bid();
            const auto ask = book.best_ask();

            if (!bid || !ask)
                return;

            const skoll::Price mark = (*bid + *ask) / 2;

            std::cout
                << "position=" << position.quantity()
                << " cash=" << position.cash()
                << " pnl=" << position.pnl(mark)
                << '\n';
        };

        auto message_handler = [&](const std::string &raw_message) {
            const auto message = skoll::feed::decode_message(raw_message);

            if (const auto *book_message =
                    std::get_if<skoll::feed::BookMessage>(&message)) {

                book.apply(*book_message);

                const auto intents = maker.quote(book);

                for (const auto &intent : intents) {
                    // replay runs the same strategy path but does not submit
                    // or cancel real orders.
                    if (!execution) {
                        auto &last_price =
                            intent.side == skoll::Side::buy
                                ? last_replay_bid
                                : last_replay_ask;

                        if (last_price && *last_price == intent.price)
                            continue;

                        std::cout
                            << "replay quote "
                            << (intent.side == skoll::Side::buy ? "buy " : "sell ")
                            << intent.quantity
                            << " @ "
                            << intent.price
                            << '\n';

                        last_price = intent.price;
                        continue;
                    }

                    // each side keeps track of its currently resting order.
                    auto &active_quote =
                        intent.side == skoll::Side::buy
                            ? active_bid
                            : active_ask;

                    // the desired quote has not changed, so leave the
                    // existing order resting.
                    if (active_quote && active_quote->price == intent.price)
                        continue;

                    try {
                        // a quote already exists at another price.
                        // cancel it before placing its replacement so that
                        // stale Maker orders do not accumulate in Valkyrie.
                        if (active_quote) {
                            execution->cancel_order(
                                config.security_id,
                                active_quote->order_id);

                            std::cout
                                << "cancelled order "
                                << active_quote->order_id
                                << '\n';

                            own_orders.erase(active_quote->order_id);
                            active_quote.reset();
                        }

                        const auto ack = execution->place_order(
                            config.security_id,
                            intent.side,
                            intent.price,
                            intent.quantity);

                        // remember both the remaining order quantity and
                        // which order is currently quoting this side.
                        own_orders[ack.order_id] = intent.quantity;

                        active_quote = ActiveQuote{
                            ack.order_id,
                            intent.price};

                        std::cout
                            << "placed order "
                            << ack.order_id
                            << ' '
                            << (intent.side == skoll::Side::buy ? "buy " : "sell ")
                            << intent.quantity
                            << " @ "
                            << intent.price
                            << '\n';

                    } catch (const std::exception &error) {
                        std::cerr
                            << "execution error: "
                            << error.what()
                            << '\n';
                    }
                }

                return;
            }

            if (const auto *trade =
                    std::get_if<skoll::feed::TradeMessage>(&message)) {

                if (auto bid = own_orders.find(trade->bid_order_id);
                    bid != own_orders.end()) {

                    position.apply_fill(
                        skoll::Side::buy,
                        trade->price,
                        trade->quantity);

                    // fully filled order is no longer an active quote.
                    // else keep track of the quantity still resting.
                    if (trade->quantity >= bid->second) {
                        own_orders.erase(bid);

                        if (active_bid &&
                            active_bid->order_id == trade->bid_order_id) {
                            active_bid.reset();
                        }
                    } else {
                        bid->second -= trade->quantity;
                    }

                    print_position();
                }

                if (auto ask = own_orders.find(trade->ask_order_id);
                    ask != own_orders.end()) {

                    position.apply_fill(
                        skoll::Side::sell,
                        trade->price,
                        trade->quantity);

                    // same lifecycle handling for the resting ask.
                    if (trade->quantity >= ask->second) {
                        own_orders.erase(ask);

                        if (active_ask &&
                            active_ask->order_id == trade->ask_order_id) {
                            active_ask.reset();
                        }
                    } else {
                        ask->second -= trade->quantity;
                    }

                    print_position();
                }
            }
        };

        std::optional<skoll::feed::Capture> capture;
        skoll::feed::Client::CaptureHandler capture_handler;

        if (config.capture_path) {
            capture.emplace(*config.capture_path);

            capture_handler =
                [&capture](const std::string_view payload) {
                    capture->write(payload);
                };
        }

        if (config.replay_path) {
            skoll::feed::Replay replay(
                *config.replay_path,
                message_handler);

            replay.run();
            return 0;
        }

        ix::initNetSystem();

        skoll::feed::Client client(
            config.ws_url,
            config.security_id,
            message_handler,
            std::move(capture_handler));

        client.run();

        ix::uninitNetSystem();

    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}