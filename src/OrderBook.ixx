module;

#include <vector>
#include <memory>
#include <iostream>
#include <type_traits>
#include <ranges>
#include <mutex>
#include <algorithm>

export module OrderBook;
import Order;

export template <typename T>
concept IsOrder = std::is_base_of_v<Order, T>;

export class OrderBook {
private:
    std::vector<std::unique_ptr<Order>> bids;
    std::vector<std::unique_ptr<Order>> asks;
    mutable std::mutex bookMutex;

    void matchOrders() {
        std::sort(bids.begin(), bids.end(), [](const auto& a, const auto& b) {
            return a->getPrice() > b->getPrice();
            });

        std::sort(asks.begin(), asks.end(), [](const auto& a, const auto& b) {
            return a->getPrice() < b->getPrice();
            });

        while (!bids.empty() && !asks.empty()) {
            auto& bestBid = bids.front();
            auto& bestAsk = asks.front();

            if (bestBid->getPrice() >= bestAsk->getPrice()) {
                double tradeQty = std::min(bestBid->getQuantity(), bestAsk->getQuantity());

                double tradePrice = bestAsk->getPrice();

                std::cout << "[MATCHING ENGINE] TRADE EXECUTED: "
                    << tradeQty << " units @ $" << tradePrice << "\n";

                bestBid->reduceQuantity(tradeQty);
                bestAsk->reduceQuantity(tradeQty);

                if (bestBid->getQuantity() <= 0.001) bids.erase(bids.begin());
                if (bestAsk->getQuantity() <= 0.001) asks.erase(asks.begin());
            }
            else {
                break;
            }
        }
    }

public:
    template <IsOrder T>
    void addOrder(std::unique_ptr<T> order) {
        std::lock_guard<std::mutex> lock(bookMutex);

        if (order->getSide() == OrderSide::Buy) {
            bids.push_back(std::move(order));
        }
        else {
            asks.push_back(std::move(order));
        }

        matchOrders();
    }

    std::vector<std::string> getAsksSnapshot() const {
        std::lock_guard<std::mutex> lock(bookMutex);
        std::vector<std::string> snapshot;
        for (const auto& ask : asks) { snapshot.push_back(ask->getOrderInfo()); }
        return snapshot;
    }

    std::vector<std::string> getBidsSnapshot() const {
        std::lock_guard<std::mutex> lock(bookMutex);
        std::vector<std::string> snapshot;
        for (const auto& bid : bids) { snapshot.push_back(bid->getOrderInfo()); }
        return snapshot;
    }
};