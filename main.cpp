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

int main() {
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

        std::unordered_map<skoll::OrderId, skoll::Side> own_orders;

        std::optional<skoll::Price> last_bid;
        std::optional<skoll::Price> last_ask;

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
                    auto &last_price =
                        intent.side == skoll::Side::buy
                            ? last_bid
                            : last_ask;

                    if (last_price && *last_price == intent.price)
                        continue;

                    if (!execution) {
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

                    try {
                        const auto ack = execution->place_order(
                            config.security_id,
                            intent.side,
                            intent.price,
                            intent.quantity
                        );

                        own_orders[ack.order_id] = intent.side;
                        last_price = intent.price;

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

                if (const auto bid = own_orders.find(trade->bid_order_id);
                    bid != own_orders.end()) {

                    position.apply_fill(
                        skoll::Side::buy,
                        trade->price,
                        trade->quantity
                    );

                    last_bid.reset();
                    print_position();
                }

                if (const auto ask = own_orders.find(trade->ask_order_id);
                    ask != own_orders.end()) {

                    position.apply_fill(
                        skoll::Side::sell,
                        trade->price,
                        trade->quantity
                    );

                    last_ask.reset();
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
                message_handler
            );

            replay.run();
            return 0;
        }

        ix::initNetSystem();

        skoll::feed::Client client(
            config.ws_url,
            config.security_id,
            message_handler,
            std::move(capture_handler)
        );

        client.run();

        ix::uninitNetSystem();

    } catch (const std::exception &error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}