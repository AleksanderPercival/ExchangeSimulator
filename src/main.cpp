#include <iostream>
#include <vector>
#include <memory>
#include <thread>

import Order;
import OrderBook;

void simulateClient(OrderBook& book, uint64_t startId, OrderSide side, int numOrders) {
    for (int i = 0; i < numOrders; ++i) {
        book.addOrder(std::make_unique<LimitOrder>(startId + i, side, 10.0, 100.0 + i));
    }
}

int main() {
    std::cout << "System initialized. Starting Exchange Simulator...\n";

    OrderBook orderBook;

    std::cout << "[SYSTEM] Client threads activation...\n";

    {
        std::jthread broker1(simulateClient, std::ref(orderBook), 10000, OrderSide::Buy, 5000);
        std::jthread broker2(simulateClient, std::ref(orderBook), 20000, OrderSide::Sell, 5000);
        std::jthread broker3(simulateClient, std::ref(orderBook), 30000, OrderSide::Buy, 5000);
    }

    std::cout << "[SYSTEM] All threads finished routing orders.\n";

    orderBook.addOrder(std::make_unique<MarketOrder>(3, OrderSide::Buy, 5000.0));

    orderBook.displayWhaleOrders(100.0);

    return 0;
}