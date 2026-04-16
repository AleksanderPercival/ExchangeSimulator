module;
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

export module OrderBook;

import Asset;
import Order;

export struct PriceLevel {
    uint32_t price;
    std::vector<Order> orders;

    explicit PriceLevel(uint32_t p) : price(p) {}
};

export class OrderBook {
private:
    Asset m_asset;
    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;

    void matchOrders() {

        while (!bids.empty() && !asks.empty()) {
            auto& bestBidLevel = bids.front();
            auto& bestAskLevel = asks.front();

            if (bestBidLevel.price < bestAskLevel.price) {
                break;
            }

            auto& bidOrder = bestBidLevel.orders.front();
            auto& askOrder = bestAskLevel.orders.front();

            uint32_t tradeQty = std::min(bidOrder.quantity, askOrder.quantity);

            std::cout << "[MATCHING ENGINE] TRADE EXECUTED: "
                << tradeQty << " units @ price " << bestAskLevel.price << "\n";

            bidOrder.quantity -= tradeQty;
            askOrder.quantity -= tradeQty;

            if (bidOrder.quantity == 0) {
                bestBidLevel.orders.erase(bestBidLevel.orders.begin());
            }
            if (askOrder.quantity == 0) {
                bestAskLevel.orders.erase(bestAskLevel.orders.begin());
            }

            if (bestBidLevel.orders.empty()) {
                bids.erase(bids.begin());
            }
            if (bestAskLevel.orders.empty()) {
                asks.erase(asks.begin());
            }
        }
    }

public:
    explicit OrderBook(Asset asset) : m_asset(std::move(asset)) {}

    const Asset& getAsset() const { return m_asset; }

    void addOrder(const Order& order) {
        if (order.side == Side::Buy) {
            auto it = std::lower_bound(bids.begin(), bids.end(), order.price,
                [](const PriceLevel& level, uint32_t price) {
                    return level.price > price;
                });

            if (it == bids.end() || it->price != order.price) {
                it = bids.insert(it, PriceLevel(order.price));
            }
            it->orders.push_back(order);
        }
        else {
            auto it = std::lower_bound(asks.begin(), asks.end(), order.price,
                [](const PriceLevel& level, uint32_t price) {
                    return level.price < price;
                });

            if (it == asks.end() || it->price != order.price) {
                it = asks.insert(it, PriceLevel(order.price));
            }
            it->orders.push_back(order);
        }

        matchOrders();
    }

    std::vector<std::string> getBidsSnapshot() const {
        std::vector<std::string> snapshot;
        for (const auto& level : bids) {
            uint32_t totalQty = 0;
            for (const auto& order : level.orders) {
                totalQty += order.quantity;
            }
            if (totalQty > 0) {
                std::string line = std::to_string(totalQty) + " units @ $" + std::to_string(level.price / 100.0f);
                snapshot.push_back(line);
            }
        }
        return snapshot;
    }

    std::vector<std::string> getAsksSnapshot() const {
        std::vector<std::string> snapshot;
        for (const auto& level : asks) {
            uint32_t totalQty = 0;
            for (const auto& order : level.orders) {
                totalQty += order.quantity;
            }
            if (totalQty > 0) {
                std::string line = std::to_string(totalQty) + " units @ $" + std::to_string(level.price / 100.0f);
                snapshot.push_back(line);
            }
        }
        return snapshot;
    }
};