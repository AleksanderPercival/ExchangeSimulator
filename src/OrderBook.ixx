module;
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <functional>

export module OrderBook;

import Asset;
import Order;
import MarketData;

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

    std::vector<Trade> tradeHistory;
    std::vector<Candle> chartData;
    uint32_t tradesInCurrentCandle = 0;

    void executeMarketOrder(Order& order) {
        if (order.side == Side::Buy) {
            while (order.quantity > 0 && !asks.empty()) {
                auto& bestAskLevel = asks.front();
                auto& askOrder = bestAskLevel.orders.front();

                uint32_t tradeQty = std::min(order.quantity, askOrder.quantity);
                std::cout << "[MATCHING ENGINE] MARKET BUY SWEEP: "
                    << tradeQty << " units @ price " << bestAskLevel.price << "\n";

                recordTrade(bestAskLevel.price, tradeQty);

                if (onTradeExecuted) {
                    onTradeExecuted(m_asset.symbol, true, bestAskLevel.price, tradeQty);
                    onTradeExecuted(m_asset.symbol, false, bestAskLevel.price, tradeQty);
                }

                order.quantity -= tradeQty;
                askOrder.quantity -= tradeQty;

                if (askOrder.quantity == 0) {
                    bestAskLevel.orders.erase(bestAskLevel.orders.begin());
                }
                if (bestAskLevel.orders.empty()) {
                    asks.erase(asks.begin());
                }
            }
            if (order.quantity > 0) {
                std::cout << "[MATCHING ENGINE] MARKET BUY PARTIALLY UNFILLED. Dropping remaining " << order.quantity << " units.\n";
            }
        }
        else {
            while (order.quantity > 0 && !bids.empty()) {
                auto& bestBidLevel = bids.front();
                auto& bidOrder = bestBidLevel.orders.front();

                uint32_t tradeQty = std::min(order.quantity, bidOrder.quantity);
                std::cout << "[MATCHING ENGINE] MARKET SELL SWEEP: "
                    << tradeQty << " units @ price " << bestBidLevel.price << "\n";

                recordTrade(bestBidLevel.price, tradeQty);

                if (onTradeExecuted) {
                    onTradeExecuted(m_asset.symbol, true, bestBidLevel.price, tradeQty);
                    onTradeExecuted(m_asset.symbol, false, bestBidLevel.price, tradeQty);
                }

                order.quantity -= tradeQty;
                bidOrder.quantity -= tradeQty;

                if (bidOrder.quantity == 0) {
                    bestBidLevel.orders.erase(bestBidLevel.orders.begin());
                }
                if (bestBidLevel.orders.empty()) {
                    bids.erase(bids.begin());
                }
            }
            if (order.quantity > 0) {
                std::cout << "[MATCHING ENGINE] MARKET SELL PARTIALLY UNFILLED. Dropping remaining " << order.quantity << " units.\n";
            }
        }
    }

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

            recordTrade(bestAskLevel.price, tradeQty);

            if (onTradeExecuted) {
                onTradeExecuted(m_asset.symbol, true, bestAskLevel.price, tradeQty);
                onTradeExecuted(m_asset.symbol, false, bestAskLevel.price, tradeQty);
            }

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

    void recordTrade(uint32_t price, uint32_t qty) {
        auto now = std::chrono::high_resolution_clock::now();
        uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

        tradeHistory.push_back(Trade{ timestamp, price, qty });

        if (chartData.empty() || tradesInCurrentCandle >= 5) {
            chartData.emplace_back(price, qty);
            tradesInCurrentCandle = 1;
        }
        else {
            chartData.back().update(price, qty);
            tradesInCurrentCandle++;
        }
    }

public:
    explicit OrderBook(Asset asset) : m_asset(std::move(asset)) {}

    const Asset& getAsset() const { return m_asset; }

    void addOrder(Order order) {
        
        if (order.type == Type::Market) {
            executeMarketOrder(order);
            return;
        }
        
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

    const std::vector<Trade>& getTradeHistory() const { return tradeHistory; }

    const std::vector<Candle>& getChartData() const { return chartData; }

    std::function<void(const std::string& symbol, bool isBuy, uint32_t price, uint32_t quantity)> onTradeExecuted;

    void setTradeCallback(std::function<void(const std::string&, bool, uint32_t, uint32_t)> cb) {
        onTradeExecuted = std::move(cb);
    }
};