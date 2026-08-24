#include "skoll/execution/client.hpp"

#include <ixwebsocket/IXHttpClient.h>
#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>
#include <utility>

namespace skoll::execution {
    namespace {
        std::string side_to_string(const Side side) {
            switch (side) {
            case Side::buy:
                return "Buy";
            case Side::sell:
                return "Sell";
            }

            throw ExecutionError("unknown side");
        }
    } // namespace

    Client::Client(std::string base_url, std::string username)
        : base_url_(std::move(base_url)),
          username_(std::move(username)) {
        while (!base_url_.empty() && base_url_.back() == '/') {
            base_url_.pop_back();
        }

        if (base_url_.empty()) {
            throw std::invalid_argument("execution base URL cannot be empty");
        }

        if (username_.empty()) {
            throw std::invalid_argument("username cannot be empty");
        }
    }

    OrderAck Client::place_order(
        const SecurityId security_id,
        const Side side,
        const Price price,
        const Quantity quantity) const {
        const nlohmann::json request = {
            {"securityId", security_id},
            {"username", username_},
            {"side", side_to_string(side)},
            {"price", price},
            {"quantity", quantity}};

        ix::HttpClient http_client;

        const auto args = http_client.createRequest();
        args->extraHeaders["Content-Type"] = "application/json";
        args->extraHeaders["Accept"] = "application/json";
        args->connectTimeout = 10;
        args->transferTimeout = 10;

        const auto response = http_client.post(
            base_url_ + "/orders",
            request.dump(),
            args);

        if (response->errorCode != ix::HttpErrorCode::Ok) {
            throw ExecutionError(
                "order request failed: " + response->errorMsg);
        }

        if (response->statusCode != 201) {
            throw ExecutionError(
                "order request returned HTTP " +
                std::to_string(response->statusCode) + ": " +
                response->body);
        }

        try {
            const auto json = nlohmann::json::parse(response->body);

            OrderAck ack;
            ack.order_id = json.at("orderId").get<OrderId>();
            ack.matched = json.at("matched").get<bool>();

            for (const auto &fill : json.at("fills")) {
                ack.fills.push_back({fill.at("bidOrderId").get<OrderId>(),
                                     fill.at("askOrderId").get<OrderId>(),
                                     fill.at("price").get<Price>(),
                                     fill.at("quantity").get<Quantity>()});
            }

            return ack;
        } catch (const nlohmann::json::exception &exception) {
            throw ExecutionError(
                "invalid order response: " + std::string(exception.what()));
        }
    }
} // namespace skoll::execution