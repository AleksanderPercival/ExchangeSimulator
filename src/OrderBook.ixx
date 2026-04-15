module;
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

        // new matchOrders() to add here
    }
};