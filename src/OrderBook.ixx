module;

#include <vector>
#include <memory>
#include <iostream>
#include <type_traits>
#include <ranges>

export module OrderBook;

import Order;

export template <typename T>
concept IsOrder = std::is_base_of_v<Order, T>;

export class OrderBook {
private:
    std::vector<std::unique_ptr<Order>> bids;
    std::vector<std::unique_ptr<Order>> asks;

public:
    template <IsOrder T>
    void addOrder(std::unique_ptr<T> order) {
        if (order->getSide() == OrderSide::Buy) {
            bids.push_back(std::move(order));
        }
        else {
            asks.push_back(std::move(order));
        }
    }

    void displayBook() const {
        std::cout << "\n=== ORDER BOOK ===\n";

        std::cout << "--- ASKS (Sell) ---\n";
        for (const auto& ask : asks) {
            std::cout << ask->getOrderInfo() << "\n";
        }

        std::cout << "--- BIDS (Buy) ---\n";
        for (const auto& bid : bids) {
            std::cout << bid->getOrderInfo() << "\n";
        }
        std::cout << "==================\n\n";
    }

    void displayWhaleOrders(double minQuantity) const {
        std::cout << "\n[RADAR] Whale Detection (Volume > " << minQuantity << ")...\n";

        auto whaleFilter = std::views::filter([minQuantity](const auto& order) {
            return order->getQuantity() > minQuantity;
            });

        for (const auto& ask : asks | whaleFilter) {
            std::cout << "[ALERT] High supply: " << ask->getOrderInfo() << "\n";
        }

        for (const auto& bid : bids | whaleFilter) {
            std::cout << "[ALERT] High demand: " << bid->getOrderInfo() << "\n";
        }
    }
};