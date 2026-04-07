module;

#include <vector>
#include <memory>
#include <iostream>
#include <type_traits>
#include <ranges>
#include <mutex>
#include <algorithm>
#include <string>

export module OrderBook;
import Order;
import MarketData;

export template <typename T>
concept IsOrder = std::is_base_of_v<Order, T>;

export class OrderBook {
private:
    std::string symbol;

    std::vector<std::unique_ptr<Order>> bids;
    std::vector<std::unique_ptr<Order>> asks;
    std::vector<std::string> tradeHistory;


    std::vector<Candle> chartData;
    int tradesInCurrentCandle = 0;

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

                std::string tradeMsg = "[TRADE] " + symbol + " - " + std::to_string(tradeQty) +
                    " units @ $" + std::to_string(tradePrice);
                std::cout << "[MATCHING ENGINE] " << tradeMsg << "\n";
                tradeHistory.push_back(tradeMsg);

                if (chartData.empty() || tradesInCurrentCandle >= 3) {
                    chartData.emplace_back(tradePrice, tradeQty);
                    tradesInCurrentCandle = 1;
                }
                else {
                    chartData.back().update(tradePrice, tradeQty);
                    tradesInCurrentCandle++;
                }

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
    OrderBook(std::string tickerSymbol) : symbol(std::move(tickerSymbol)) {}

    std::string getSymbol() const { return symbol; }

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

    std::vector<std::string> getTradeHistorySnapshot() const {
        std::lock_guard<std::mutex> lock(bookMutex);
        return tradeHistory;
    }

    std::vector<Candle> getChartDataSnapshot() const {
        std::lock_guard<std::mutex> lock(bookMutex);
        return chartData;
    }
};