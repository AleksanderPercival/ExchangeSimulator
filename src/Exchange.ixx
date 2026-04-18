module;

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

export module Exchange;

import Asset;
import Order;
import OrderBook;

export class Exchange {
private:
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> orderBooks;

public:
    void addAsset(const Asset& asset) {
        orderBooks[asset.symbol] = std::make_unique<OrderBook>(asset);
        std::cout << "[EXCHANGE] Listed new asset: " << asset.symbol << "\n";
    }

    OrderBook* getOrderBook(const std::string& symbol) {
        auto it = orderBooks.find(symbol);
        if (it != orderBooks.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void routeOrder(const std::string& symbol, Order order) {
        OrderBook* book = getOrderBook(symbol);
        if (book) {
            book->addOrder(order);
        }
        else {
            std::cout << "[EXCHANGE] CRITICAL ERROR: Order routed to unknown symbol: " << symbol << "\n";
        }
    }
};